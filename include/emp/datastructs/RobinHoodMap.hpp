/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/RobinHoodMap.hpp
 * @brief Robin Hood hashing - minimize how "wrong" a position can be in a hash table.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD

#include <algorithm>   // for std::max
#include <compare>     // for operator<=> helpers
#include <cstddef>     // for std::ptrdiff_t
#include <functional>  // for std::hash
#include <iostream>
#include <iterator>    // for iterator tags
#include <optional>
#include <print>
#include <string>
#include <type_traits> // for std::conditional_t
#include <utility>     // for std::swap

#include "../base/vector.hpp"
#include "../meta/type_traits.hpp"

namespace emp {

  template <typename KEY_T, typename MAPPED_T, bool IMPROVE_HASH=false>
  class RobinHoodMap {
  private:
    struct Entry {
      KEY_T first;
      MAPPED_T second;

      [[nodiscard]] KEY_T & Key() { return first; }
      [[nodiscard]] const KEY_T & Key() const { return first; }
      [[nodiscard]] MAPPED_T & Value() { return second; }
      [[nodiscard]] const MAPPED_T & Value() const { return second; }

      // Implicit conversion to std::pair for algorithms that expect value_type.
      operator std::pair<const KEY_T, MAPPED_T>() const { return { first, second }; }
    };

    static constexpr size_t INIT_CAPACITY = 16;
    static constexpr size_t GROW_FACTOR = 2; // How much larger should table get on growth?
    static constexpr size_t LOAD_FACTOR = 2; // Max factor for how full a table can get.

    emp::vector<Entry> table = emp::vector<Entry>(INIT_CAPACITY);
    emp::vector<size_t> hash_cache = emp::vector<size_t>(INIT_CAPACITY, 0);
    size_t table_mask = INIT_CAPACITY - 1;

    // Track "distance from home" of each table entry; 0=no entry; 1=in place; 2+=dist out of place
    emp::vector<uint8_t> occupied = emp::vector<uint8_t>(INIT_CAPACITY, 0);
    size_t num_elements = 0;

    // === HELPER FUNCTIONS ===

    [[nodiscard]] size_t ToPos(size_t hash) const { return hash & table_mask; }

    [[nodiscard]] static size_t ImproveHash(size_t hash_value) {
      // SplitMix64 finalizer (good quality, cheap hash improver)
      hash_value += 0x9e3779b97f4a7c15ull;
      hash_value = (hash_value ^ (hash_value >> 30)) * 0xbf58476d1ce4e5b9ull;
      hash_value = (hash_value ^ (hash_value >> 27)) * 0x94d049bb133111ebull;
      hash_value = (hash_value ^ (hash_value >> 31));
      return hash_value;
    }
  
    [[nodiscard]] static size_t CalcHash(const KEY_T & key) {
      const size_t hash_val = std::hash<KEY_T>{}(key);
      if constexpr (IMPROVE_HASH) {
        return ImproveHash(hash_val);
      } else {
        return hash_val;
      }
    }

    void Rehash(size_t new_table_size) {
      emp_assert(OK());
      emp::vector<Entry> old_table = std::move(table);          // Back up the current table
      emp::vector<uint8_t> old_occupied = std::move(occupied);  // Back up the occupied tracker
      table = emp::vector<Entry>(new_table_size);               // Build a new table with the larger size
      hash_cache = emp::vector<size_t>(new_table_size, 0);      // Build a new cache for storing hash values
      occupied = emp::vector<uint8_t>(new_table_size, 0);       // Build a new set of occupied elements
      num_elements = 0;                                         // Reset number of elements to zero before reinsertion
      table_mask = new_table_size - 1;                          // Update the table mask for the new table size

      // Restore all elements into the new hash table.
      for (size_t i = 0; i < old_occupied.size(); ++i) {
        if (old_occupied[i]) { Insert(old_table[i].Key(), old_table[i].Value()); }
      }
      emp_assert(OK());
    }

    struct SearchPos {
      size_t hash;
      size_t pos;
      uint8_t dist;

      void Next(const size_t table_mask) {
        pos = (pos + 1) & table_mask;
        ++dist;
        emp_assert(dist < 255);
      }
      operator size_t() const { return pos; }
    };

    [[nodiscard]] SearchPos MakeSearchPos(const KEY_T & key) const {
      const size_t hash = CalcHash(key);
      return SearchPos{hash, ToPos(hash), 1};
    }

    // Test if a key is found at a given search position.
    [[nodiscard]] bool TestAt(const SearchPos & test_pos, const KEY_T & key) const {
      return hash_cache[test_pos.pos] == test_pos.hash && table[test_pos.pos].Key() == key;
    }

    [[nodiscard]] std::optional<size_t> FindIndex(const KEY_T & key) const {
      if (table.size() == 0) return {}; // Nothing in table!
      SearchPos test_pos{MakeSearchPos(key)};

      while (test_pos.dist <= occupied[test_pos]) {
        if (TestAt(test_pos, key)) { return test_pos.pos; } // Found!
        test_pos.Next(table_mask); // Try the next position.
      }
      return {}; // Not found!
    }

    // Erase the element at a specified position in the table and do backshift
    void EraseAt(size_t pos) {
      emp_assert(OK());
      emp_assert(pos < table.size());
      size_t next = ToPos(pos + 1);
      while (occupied[next] > 1) { // While next entry should move up...
        table[pos] = std::move(table[next]);
        hash_cache[pos] = hash_cache[next];
        occupied[pos] = occupied[next] - 1;
        pos = next;
        next = ToPos(next + 1);
      }

      // Clear final position
      occupied[pos] = 0;
      --num_elements;
      emp_assert(OK());
    }

  public:
    RobinHoodMap() = default;
    RobinHoodMap(const RobinHoodMap & other) = default;
    RobinHoodMap(RobinHoodMap && other) noexcept
      : table(std::move(other.table))
      , hash_cache(std::move(other.hash_cache))
      , table_mask(other.table_mask)
      , occupied(std::move(other.occupied))
      , num_elements(other.num_elements)
    {
      if (table.empty()) { // In case we are moving in an already moved-from object.
        table.resize(INIT_CAPACITY);
        hash_cache.resize(INIT_CAPACITY);
        occupied.resize(INIT_CAPACITY, 0);
        table_mask = INIT_CAPACITY-1;
      }
      other.num_elements = 0;
      emp_assert(OK());
    }

    RobinHoodMap & operator=(const RobinHoodMap & other) = default;
    RobinHoodMap & operator=(RobinHoodMap && other) noexcept {
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
        table_mask = INIT_CAPACITY-1;
      }
      other.num_elements = 0;
      emp_assert(OK());
      return *this;
    }

    ~RobinHoodMap() = default;

    // === ITERATOR TYPES ===

    template <bool IS_CONST>
    class iterator_base {
    private:
      friend class RobinHoodMap;

      // Build types based on if this is a const iterator.
      using map_t = std::conditional_t<IS_CONST, const RobinHoodMap, RobinHoodMap>;
  
      map_t * map_ptr = nullptr;  // Map that owns this iterator
      size_t index      = 0;      // Current index in the table

      bool MakeValid() {
        if (!map_ptr) return false; // Cannot make valid.
        const size_t table_size = map_ptr->table.size();
        while (index < table_size && !map_ptr->occupied[index]) {
          ++index;
        }
        return true;
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = std::pair<const KEY_T, MAPPED_T>;
      using difference_type   = std::ptrdiff_t;
      using reference         = Entry;
      using pointer           = void;  // no real pointer type

      iterator_base() = default;
      iterator_base(map_t * map_ptr, size_t index=0) : map_ptr(map_ptr), index(index) {
        MakeValid();
      }
      iterator_base(const iterator_base &) = default;
      iterator_base & operator=(const iterator_base &) = default;
      ~iterator_base() = default;

      bool IsValid() const {
        return map_ptr &&                        // Does map exist?
               index < map_ptr->table.size() &&  // With the needed index?
               map_ptr->occupied[index];         // And that index is occupied?
      }

      const reference & operator*() const {
        emp_assert(IsValid());
        return map_ptr->table[index];
      }

      const Entry * operator->() const {
        emp_assert(IsValid());
        return &map_ptr->table[index];
      }

      Entry * operator->() {
        emp_assert(IsValid());
        return &map_ptr->table[index];
      }

      iterator_base & operator++() {
        ++index;
        MakeValid();
        return *this;
      }

      iterator_base operator++(int) {
        iterator_base tmp = *this;
        ++(*this);
        return tmp;
      }

      [[nodiscard]] constexpr auto operator<=>(const iterator_base &) const = default;
    };

    using iterator        = iterator_base<false>;
    using const_iterator  = iterator_base<true>;
    using key_type        = KEY_T;
    using mapped_type     = MAPPED_T;
    using value_type      = std::pair<const KEY_T, MAPPED_T>;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

    [[nodiscard]] size_t size() const { return num_elements; }

    [[nodiscard]] size_t bucket_count() const { return table.size(); }

    [[nodiscard]] size_t max_load() const { return table.size() / LOAD_FACTOR; }

    [[nodiscard]] bool empty() const { return num_elements == 0; }

    [[nodiscard]] bool contains(const KEY_T & key) const { return FindPtr(key) != nullptr; }

    [[nodiscard]] iterator begin() { return iterator(this, 0); }
    [[nodiscard]] iterator end() { return iterator(this, table.size()); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator end() const { return const_iterator(this, table.size()); }
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(this, table.size()); }

    void clear() {
      emp_assert(OK());
      for (auto & cur_occupied : occupied) cur_occupied = 0;
      num_elements = 0;
      emp_assert(OK());
    }

    // Make sure that we can store at least n elements without additional allocations.
    void reserve(size_t n) {
      emp_assert(OK());
      // If we already have enough capacity, do nothing.
      if (n <= max_load()) return;

      // Grow with the same rule used in Insert
      size_t new_max_load = std::max(table.size(), INIT_CAPACITY);
      while (new_max_load < n) new_max_load *= GROW_FACTOR;

      Rehash(new_max_load * LOAD_FACTOR);
      emp_assert(OK());
    }

    std::pair<iterator,bool> insert(const std::pair<const KEY_T, MAPPED_T> & in) {
      return Insert(in.first, in.second);
    }

    /// Insert with "unique key" semantics.
    /// @return pair of iterator to key and bool to indicate if a new element was inserted.
    std::pair<iterator,bool> Insert(const KEY_T & key, const MAPPED_T & value) {
      emp_assert(OK());
      // Test if we need to grow the table...
      if (num_elements > max_load()) { Rehash(table.size() * GROW_FACTOR); }

      // Search for an existing key. Return iterator/false if we find one; else end loop.
      SearchPos test_pos{MakeSearchPos(key)};
      while (occupied[test_pos]) {        
        if (TestAt(test_pos, key)) {
          return { iterator{this, test_pos}, false };  // KEY_T already in table.
        }

        // If current is "closer to home" than insert would be, break to start Robin Hood swap-in.
        if (test_pos.dist > occupied[test_pos]) break;

        test_pos.Next(table_mask); // Keep searching at the next position.
      }

      // Current key not in table; perform standard Robin Hood insertion from found position.
      Entry new_entry{key, value};
      size_t new_hash{test_pos.hash};
      size_t found_pos{test_pos};

      // Search for an empty position, juggling entries as we go.
      while (occupied[test_pos]) {
        // Bump entry closer to home.
        if (occupied[test_pos] < test_pos.dist) {
          std::swap(table[test_pos], new_entry);
          std::swap(hash_cache[test_pos], new_hash);
          std::swap(test_pos.dist, occupied[test_pos]);
        }

        test_pos.Next(table_mask);  // Move on to the next table position.
      }

      table[test_pos] = std::move(new_entry);
      hash_cache[test_pos] = new_hash;
      occupied[test_pos] = test_pos.dist;
      ++num_elements;

      emp_assert(OK());
      return { iterator{this, found_pos}, true };
    }

    template <class Self>
    [[nodiscard]] auto * FindPtr(this Self & self, const KEY_T & key) {
      if (self.table.size() > 0) {
        SearchPos test_pos{self.MakeSearchPos(key)};

        while (test_pos.dist <= self.occupied[test_pos]) {
          if (self.TestAt(test_pos, key)) { return &self.table[test_pos].Value(); } // Found!
          test_pos.Next(self.table_mask); // Try the next position.
        }
      }
      using base_t = emp::match_const_t<MAPPED_T, Self>;
      return static_cast<base_t *>(nullptr); // Not found!
    }

    // Find and return an object by value; may provide a default for "not found"
    [[nodiscard]] MAPPED_T FindValue(const KEY_T & key, MAPPED_T default_obj={}) const {
      const MAPPED_T * ptr = FindPtr(key);
      if (ptr) return *ptr;
      return default_obj;
    }

    MAPPED_T & operator[](const KEY_T & key) {
      MAPPED_T * val = FindPtr(key);
      if (val) { return *val; }

      Insert(key, MAPPED_T{});
      return *FindPtr(key);
    }

    // Standard-library compatible find()
    [[nodiscard]] iterator find(const KEY_T & key) {
      std::optional<size_t> index = FindIndex(key);
      if (index) return iterator{this, *index};
      return end();
    }

    // Standard-library compatible const find()
    [[nodiscard]] const_iterator find(const KEY_T & key) const {
      std::optional<size_t> index = FindIndex(key);
      if (index) return const_iterator{this, *index};
      return end();
    }

    bool erase(const KEY_T & key) {
      emp_assert(OK());
      if (table.empty()) { return false; }  // Nothing to delete.

      SearchPos test_pos{MakeSearchPos(key)};
      while (occupied[test_pos] >= test_pos.dist) {
        if (TestAt(test_pos, key)) {  // If we found key, begin deletion and backshift
          EraseAt(test_pos);
          return true;
        }

        test_pos.Next(table_mask);
      }

      emp_assert(OK());
      return false;  // Couldn't find in table.
    }

    // Calculate how far off a current entry is from its ideal position.
    [[nodiscard]] size_t CalcOffset(size_t pos) const {
      emp_assert(pos < table.size() && occupied[pos], pos, table.size());
      // Note: We don't need to hash % table.size() before subtraction because table is power of two.
      return ToPos(pos - hash_cache[pos]) + 1;
    }

    /////////////////////////////////////////
    // Container analysis tools

    // Identify the average distance that entries have to be pushed from their "home" position.
    [[nodiscard]] double CalcAveOffset() const {
      double total = 0.0;
      for (uint8_t offset : occupied) {
        if (offset) total += (offset - 1);
      }
      return total / num_elements;
    }

    // Calculate how many 1's there are at each bit position across hashes.
    [[nodiscard]] emp::vector<size_t> CalcHashSums() const {
      // Count the number of bits we are using.
      size_t hash_bits = 0;
      while (table_mask >> hash_bits) ++hash_bits;

      // Count the number of ones at each bit position.
      emp::vector<size_t> one_counts(hash_bits, 0);
      for (size_t i = 0; i < occupied.size(); ++i) {
        if (!occupied[i]) continue;
        for (size_t pos = 0; pos < hash_bits; ++pos) {
          if (hash_cache[i] & (size_t{1} << pos)) one_counts[pos]++;
        }
      }

      return one_counts;
    }

    // Print to the provided stream a string describing the quality of the hash function.
    void EvalHashQuality(std::ostream & os = std::cout) const {
      if (num_elements < 100) {
        std::println("Poor hash quality evaluation with < 100 entries; {} found", num_elements);
      }
      emp::vector<size_t> one_counts = CalcHashSums();

      // Identify bits that are all the same across hashes (always zero or 1).
      int locked_bits = 0;
      std::string pattern;
      for (size_t cur_count : one_counts) {
        if (cur_count == 0) { ++locked_bits; pattern = "0" + pattern; }
        else if (cur_count == num_elements) { ++locked_bits; pattern = "1" + pattern; }
        else pattern = "." + pattern;
      }
      if (locked_bits) {
        // os << locked_bits << " bits are locked.  Pattern: " << pattern << std::endl;
        std::println(os, "{} bits are locked.  Pattern: {}", locked_bits, pattern);
      }

      // Identify the number of duplicate hashes.
      RobinHoodMap<size_t, size_t> hash_counts;
      for (size_t i = 0; i < hash_cache.size(); ++i) {
        if (!occupied[i]) continue;
        hash_counts[hash_cache[i]]++;
      }

      double ratio = hash_counts.size() / static_cast<double>(num_elements);
      std::println(os, "Full hash duplication fraction = {} (lower is better)", 1.0 - ratio);
      std::println(os, "Average entry offset = {} (lower is better)", CalcAveOffset());
    } 

    [[nodiscard]] bool OK() const {
      const size_t N = table.size();

      // Make sure the hash cache and occupied sizes are the same as the table.
      emp_assert(hash_cache.size() == N, hash_cache.size(), table.size());
      emp_assert(occupied.size() == N, occupied.size(), table.size());

      // Make sure the size is valid (at least minumum and a power of two.)
      emp_assert((N >= INIT_CAPACITY) && ((N & (N - 1)) == 0));

      // Make sure we have kept the mask up to date.
      emp_assert(table_mask == N - 1, table_mask, table.size());

      // Make sure we have the correct number of occupied bins.
      [[maybe_unused]] size_t entry_count = 0;
      for (uint8_t cur_oc : occupied) { if (cur_oc) ++entry_count; }
      emp_assert(entry_count == num_elements, entry_count, num_elements);

      // Make sure all hash values are correct
      for (size_t id = 0; id < N; ++id) {
        if (!occupied[id]) continue;
        emp_assert(hash_cache[id] == CalcHash(table[id].Key()));
      }

      // Test that distances match expectations.
      for (size_t i = 0; i < N; ++i) {
        if (!occupied[i]) continue;

        [[maybe_unused]] const size_t cur_dist = ToPos(i - hash_cache[i]) + 1;

        // Distance at next slot (if occupied) must be <= ours + 1 (or we should have swapped)
        size_t j = (i + 1) & table_mask;
        if (occupied[j]) {
          [[maybe_unused]] const size_t next_dist = ToPos(j - hash_cache[j]) + 1;
          emp_assert(next_dist <= cur_dist + 1, i, cur_dist, j, next_dist);
        }
      }

      return true;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
