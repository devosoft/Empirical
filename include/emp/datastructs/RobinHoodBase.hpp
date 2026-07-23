/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/RobinHoodBase.hpp
 * @brief Shared Robin Hood hash-table mechanics for RobinHoodMap and RobinHoodSet.
 * @note Status: ALPHA
 *
 * ENTRY_T requirements:
 *   KEY_T &       Key()            — mutable key access
 *   const KEY_T & Key() const      — const key access
 *   static auto & ToRef(ENTRY_T &)            — returns iterator reference (mutable for map,
 *   static auto & ToRef(const ENTRY_T &)        const for set)
 *   using value_type = ...         — the STL-facing value type (pair<K,V> for map, K for set)
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_BASE_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_BASE_HPP_GUARD

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <print>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "../base/assert.hpp"
#include "../base/vector.hpp"

namespace emp {

  template <typename KEY_T, typename ENTRY_T, bool IMPROVE_HASH = false>
  class RobinHoodBase {
  protected:
    static constexpr size_t INIT_CAPACITY = 16;
    static constexpr size_t GROW_FACTOR   = 2;
    static constexpr size_t LOAD_FACTOR   = 2;

    emp::vector<ENTRY_T>  table        = emp::vector<ENTRY_T>(INIT_CAPACITY);
    emp::vector<size_t>   hash_cache   = emp::vector<size_t>(INIT_CAPACITY, 0);
    size_t                table_mask   = INIT_CAPACITY - 1;
    emp::vector<uint8_t>  occupied     = emp::vector<uint8_t>(INIT_CAPACITY, 0);
    size_t                num_elements = 0;

    // === INTERNAL HELPERS ===

    [[nodiscard]] size_t ToPos(size_t hash) const noexcept { return hash & table_mask; }

    [[nodiscard]] static size_t ImproveHash(size_t h) noexcept {
      h += 0x9e3779b97f4a7c15ull;
      h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9ull;
      h = (h ^ (h >> 27)) * 0x94d049bb133111ebull;
      return h ^ (h >> 31);
    }

    [[nodiscard]] static size_t CalcHash(const KEY_T & key) {
      const size_t h = std::hash<KEY_T>{}(key);
      if constexpr (IMPROVE_HASH) return ImproveHash(h);
      return h;
    }

    struct SearchPos {
      size_t  hash;
      size_t  pos;
      uint8_t dist;

      void Next(size_t mask) noexcept {
        pos = (pos + 1) & mask;
        ++dist;
        emp_assert(dist < 255, dist);
      }
      operator size_t() const noexcept { return pos; }
    };

    [[nodiscard]] SearchPos MakeSearchPos(const KEY_T & key) const {
      const size_t h = CalcHash(key);
      return SearchPos{h, ToPos(h), 1};
    }

    [[nodiscard]] bool TestAt(const SearchPos & sp, const KEY_T & key) const {
      return hash_cache[sp.pos] == sp.hash && table[sp.pos].Key() == key;
    }

    [[nodiscard]] std::optional<size_t> FindIndex(const KEY_T & key) const {
      if (table.empty()) return {};
      SearchPos sp{MakeSearchPos(key)};
      while (sp.dist <= occupied[sp]) {
        if (TestAt(sp, key)) return sp.pos;
        sp.Next(table_mask);
      }
      return {};
    }

    void EraseAt(size_t pos) {
      size_t next = ToPos(pos + 1);
      while (occupied[next] > 1) {
        table[pos]      = std::move(table[next]);
        hash_cache[pos] = hash_cache[next];
        occupied[pos]   = occupied[next] - 1;
        pos  = next;
        next = ToPos(next + 1);
      }
      occupied[pos] = 0;
      --num_elements;
    }

    void Rehash(size_t new_table_size) {
      emp_assert(OK());
      emp::vector<ENTRY_T>  old_table    = std::move(table);
      emp::vector<uint8_t>  old_occupied = std::move(occupied);
      table        = emp::vector<ENTRY_T>(new_table_size);
      hash_cache   = emp::vector<size_t>(new_table_size, 0);
      occupied     = emp::vector<uint8_t>(new_table_size, 0);
      num_elements = 0;
      table_mask   = new_table_size - 1;
      for (size_t i = 0; i < old_occupied.size(); ++i) {
        if (old_occupied[i]) InsertEntry(std::move(old_table[i]));
      }
      emp_assert(OK());
    }

    // Core Robin Hood insertion.  Returns {slot_of_key, was_inserted}.
    std::pair<size_t, bool> InsertEntry(ENTRY_T new_entry) {
      if (num_elements > max_load()) Rehash(table.size() * GROW_FACTOR);

      SearchPos sp{MakeSearchPos(new_entry.Key())};
      while (occupied[sp]) {
        if (TestAt(sp, new_entry.Key())) return {(size_t)sp, false};
        if (sp.dist > occupied[sp]) break;
        sp.Next(table_mask);
      }

      const size_t found_pos = sp;
      size_t new_hash = sp.hash;

      while (occupied[sp]) {
        if (occupied[sp] < sp.dist) {
          std::swap(table[sp],      new_entry);
          std::swap(hash_cache[sp], new_hash);
          std::swap(sp.dist,        occupied[sp]);
        }
        sp.Next(table_mask);
      }

      table[sp]      = std::move(new_entry);
      hash_cache[sp] = new_hash;
      occupied[sp]   = sp.dist;
      ++num_elements;
      return {found_pos, true};
    }

  public:
    RobinHoodBase() = default;
    RobinHoodBase(const RobinHoodBase &) = default;
    RobinHoodBase(RobinHoodBase && other) noexcept
      : table(std::move(other.table))
      , hash_cache(std::move(other.hash_cache))
      , table_mask(other.table_mask)
      , occupied(std::move(other.occupied))
      , num_elements(other.num_elements)
    {
      if (table.empty()) {
        table.resize(INIT_CAPACITY);
        hash_cache.resize(INIT_CAPACITY);
        occupied.resize(INIT_CAPACITY, 0);
        table_mask = INIT_CAPACITY - 1;
      }
      other.num_elements = 0;
    }

    RobinHoodBase & operator=(const RobinHoodBase &) = default;
    RobinHoodBase & operator=(RobinHoodBase && other) noexcept {
      if (this == &other) return *this;
      table        = std::move(other.table);
      hash_cache   = std::move(other.hash_cache);
      table_mask   = other.table_mask;
      occupied     = std::move(other.occupied);
      num_elements = other.num_elements;
      if (table.empty()) {
        table.resize(INIT_CAPACITY);
        hash_cache.resize(INIT_CAPACITY);
        occupied.resize(INIT_CAPACITY, 0);
        table_mask = INIT_CAPACITY - 1;
      }
      other.num_elements = 0;
      return *this;
    }

    ~RobinHoodBase() = default;

    // === ITERATOR ===

    template <bool IS_CONST>
    class iterator_base {
    private:
      using base_t = std::conditional_t<IS_CONST, const RobinHoodBase, RobinHoodBase>;
      base_t * base_ptr = nullptr;
      size_t   index    = 0;

      bool MakeValid() noexcept {
        if (!base_ptr) return false;
        const size_t sz = base_ptr->table.size();
        while (index < sz && !base_ptr->occupied[index]) ++index;
        return true;
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = typename ENTRY_T::value_type;
      using difference_type   = std::ptrdiff_t;

      iterator_base() = default;
      iterator_base(base_t * p, size_t i = 0) : base_ptr(p), index(i) { MakeValid(); }
      iterator_base(const iterator_base &)            = default;
      iterator_base & operator=(const iterator_base &) = default;
      ~iterator_base()                                = default;

      [[nodiscard]] bool IsValid() const noexcept {
        return base_ptr &&
               index < base_ptr->table.size() &&
               base_ptr->occupied[index];
      }

      // operator* and operator-> return whatever ENTRY_T::ToRef maps to.
      // For map: mutable Entry & / const Entry & depending on IS_CONST.
      // For set: always const KEY_T &.
      [[nodiscard]] decltype(auto) operator*() const {
        emp_assert(IsValid());
        return ENTRY_T::ToRef(base_ptr->table[index]);
      }

      [[nodiscard]] auto * operator->() const {
        emp_assert(IsValid());
        return &ENTRY_T::ToRef(base_ptr->table[index]);
      }

      iterator_base & operator++() noexcept { ++index; MakeValid(); return *this; }
      iterator_base   operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }

      [[nodiscard]] constexpr auto operator<=>(const iterator_base &) const = default;
    };

    using iterator        = iterator_base<false>;
    using const_iterator  = iterator_base<true>;
    using key_type        = KEY_T;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

    [[nodiscard]] size_t size()         const noexcept { return num_elements; }
    [[nodiscard]] size_t bucket_count() const noexcept { return table.size(); }
    [[nodiscard]] size_t max_load()     const noexcept { return table.size() / LOAD_FACTOR; }
    [[nodiscard]] bool   empty()        const noexcept { return num_elements == 0; }

    [[nodiscard]] bool contains(const KEY_T & key) const { return FindIndex(key).has_value(); }

    [[nodiscard]] iterator       begin()        { return {this, 0}; }
    [[nodiscard]] iterator       end()          { return {this, table.size()}; }
    [[nodiscard]] const_iterator begin()  const { return {this, 0}; }
    [[nodiscard]] const_iterator end()    const { return {this, table.size()}; }
    [[nodiscard]] const_iterator cbegin() const { return {this, 0}; }
    [[nodiscard]] const_iterator cend()   const { return {this, table.size()}; }

    void clear() noexcept {
      for (auto & o : occupied) o = 0;
      num_elements = 0;
    }

    void reserve(size_t n) {
      if (n <= max_load()) return;
      size_t new_max = std::max(table.size(), INIT_CAPACITY);
      while (new_max < n) new_max *= GROW_FACTOR;
      Rehash(new_max * LOAD_FACTOR);
    }

    bool erase(const KEY_T & key) {
      if (table.empty()) return false;
      SearchPos sp{MakeSearchPos(key)};
      while (occupied[sp] >= sp.dist) {
        if (TestAt(sp, key)) { EraseAt(sp); return true; }
        sp.Next(table_mask);
      }
      return false;
    }

    [[nodiscard]] iterator find(const KEY_T & key) {
      auto idx = FindIndex(key);
      return idx ? iterator{this, *idx} : end();
    }

    [[nodiscard]] const_iterator find(const KEY_T & key) const {
      auto idx = FindIndex(key);
      return idx ? const_iterator{this, *idx} : end();
    }

    /////////////////////////////////////////
    // Container analysis tools

    [[nodiscard]] size_t CalcOffset(size_t pos) const {
      emp_assert(pos < table.size() && occupied[pos], pos, table.size());
      return ToPos(pos - hash_cache[pos]) + 1;
    }

    [[nodiscard]] double CalcAveOffset() const {
      double total = 0.0;
      for (uint8_t o : occupied) if (o) total += (o - 1);
      return total / num_elements;
    }

    [[nodiscard]] emp::vector<size_t> CalcHashSums() const {
      size_t hash_bits = 0;
      while (table_mask >> hash_bits) ++hash_bits;
      emp::vector<size_t> ones(hash_bits, 0);
      for (size_t i = 0; i < occupied.size(); ++i) {
        if (!occupied[i]) continue;
        for (size_t b = 0; b < hash_bits; ++b)
          if (hash_cache[i] & (size_t{1} << b)) ones[b]++;
      }
      return ones;
    }

    void EvalHashQuality(std::ostream & os = std::cout) const {
      if (num_elements < 100)
        std::println(os, "Poor hash quality evaluation with < 100 entries; {} found", num_elements);

      emp::vector<size_t> one_counts = CalcHashSums();
      int locked = 0;
      std::string pattern;
      for (size_t c : one_counts) {
        if      (c == 0)            { ++locked; pattern = "0" + pattern; }
        else if (c == num_elements) { ++locked; pattern = "1" + pattern; }
        else                                    pattern = "." + pattern;
      }
      if (locked) std::println(os, "{} bits are locked.  Pattern: {}", locked, pattern);

      std::unordered_map<size_t, size_t> hash_counts;
      for (size_t i = 0; i < hash_cache.size(); ++i) {
        if (occupied[i]) hash_counts[hash_cache[i]]++;
      }
      const double ratio = hash_counts.size() / static_cast<double>(num_elements);
      std::println(os, "Full hash duplication fraction = {} (lower is better)", 1.0 - ratio);
      std::println(os, "Average entry offset = {} (lower is better)", CalcAveOffset());
    }

    [[nodiscard]] bool OK() const {
      const size_t N = table.size();
      emp_assert(hash_cache.size() == N, hash_cache.size(), N);
      emp_assert(occupied.size()   == N, occupied.size(),   N);
      emp_assert((N >= INIT_CAPACITY) && ((N & (N - 1)) == 0));
      emp_assert(table_mask == N - 1, table_mask, N);

      [[maybe_unused]] size_t cnt = 0;
      for (uint8_t o : occupied) if (o) ++cnt;
      emp_assert(cnt == num_elements, cnt, num_elements);

      for (size_t id = 0; id < N; ++id) {
        if (!occupied[id]) continue;
        emp_assert(hash_cache[id] == CalcHash(table[id].Key()));
      }
      for (size_t i = 0; i < N; ++i) {
        if (!occupied[i]) continue;
        [[maybe_unused]] const size_t cur_dist = ToPos(i - hash_cache[i]) + 1;
        const size_t j = (i + 1) & table_mask;
        if (occupied[j]) {
          [[maybe_unused]] const size_t next_dist = ToPos(j - hash_cache[j]) + 1;
          emp_assert(next_dist <= cur_dist + 1, i, cur_dist, j, next_dist);
        }
      }
      return true;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_BASE_HPP_GUARD
