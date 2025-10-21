/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/RobinHoodMap.hpp
 * @brief Robin Hood hashing - minimize how "wrong" a position can be in a hash table.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD

#include "../base/vector.hpp"

namespace emp {

  template <typename Key, typename T, bool IMPROVE_HASH=false>
  class RobinHoodMap {
  private:
    struct Entry {
      Key key;
      T value;
      size_t hash = 0;        // Store hash to avoid recomputing
      bool occupied = false;  // @CAO Optimize away later

      operator bool() const { return occupied; }
    };

    static constexpr size_t INIT_CAPACITY = 16;
    static constexpr size_t GROW_FACTOR = 2; // How much larger should table get on growth?
    static constexpr size_t LOAD_FACTOR = 2; // Max factor for how much of table can be full.

    emp::vector<Entry> table{INIT_CAPACITY};
    size_t num_elements = 0;

    // === HELPER FUNCTIONS ===

    [[nodiscard]] size_t ToPos(size_t hash) const { return hash & (table.size() - 1); }

    [[nodiscard]] static size_t ImproveHash(size_t hash_value) {
      // SplitMix64 finalizer (good quality, cheap enough for a hash table)
      hash_value += 0x9e3779b97f4a7c15ull;
      hash_value = (hash_value ^ (hash_value >> 30)) * 0xbf58476d1ce4e5b9ull;
      hash_value = (hash_value ^ (hash_value >> 27)) * 0x94d049bb133111ebull;
      hash_value = (hash_value ^ (hash_value >> 31));
      return hash_value;
    }
  
    [[nodiscard]] static size_t CalcHash(const Key & key) {
      const size_t hash_val = std::hash<Key>{}(key);
      if constexpr (IMPROVE_HASH) {
        return ImproveHash(hash_val);
      } else {
        return hash_val;
      }
    }

    // Calculate how far off a current entry is from its ideal position.
    [[nodiscard]] size_t CalcOffset(size_t pos) const {
      emp_assert(pos < table.size() && table[pos].occupied, pos, table.size());
      const size_t ideal_pos = ToPos(table[pos].hash);
      return ToPos(pos + table.size() - ideal_pos);
    }

    void Rehash(size_t new_table_size) {
      emp::vector<Entry> old_table = std::move(table);
      table = emp::vector<Entry>(new_table_size);
      num_elements = 0;

      // Move all elements into the new hash table.
      for (const Entry & entry : old_table) {
        if (entry) { Insert(entry.key, entry.value); }
      }
    }

    struct SearchPos {
      size_t hash;
      size_t pos;
      size_t dist;

      void Next(const size_t table_size) {
        pos = (pos + 1) & (table_size - 1);
        ++dist;
      }
      operator size_t() const { return pos; }
    };

    [[nodiscard]] SearchPos MakeSearchPos(const Key & key) const {
      const size_t hash = CalcHash(key);
      return SearchPos{hash, ToPos(hash), 0};
    }

    [[nodiscard]] bool TestAt(const SearchPos & test_pos, const Key & key) const {
      return table[test_pos.pos].hash == test_pos.hash && table[test_pos.pos].key == key;
    }

    [[nodiscard]] const T * FindPtr_impl(const Key & key) const {
      if (table.size() == 0) return nullptr;
      SearchPos test_pos{MakeSearchPos(key)};

      while (table[test_pos] && test_pos.dist <= CalcOffset(test_pos)) {
        if (TestAt(test_pos, key)) { return &table[test_pos].value; } // Found!
        test_pos.Next(table.size()); // Try the next position.
      }
      return nullptr; // Not found!
    }

    // Erase the element at a specified position in the table and do backshift
    void EraseAt(size_t pos) {
      emp_assert(pos < table.size());
      size_t next = ToPos(pos + 1);
      while (table[next] && CalcOffset(next) != 0) { // Test if next entry should move up.
        table[pos] = std::move(table[next]);
        pos = next;
        next = ToPos(next + 1);
      }

      // Clear final position
      table[pos].occupied = false;
      --num_elements;
    }

  public:
    RobinHoodMap() = default;
    RobinHoodMap(const RobinHoodMap & other) = default;
    RobinHoodMap(RobinHoodMap && other) noexcept
      : table(std::move(other.table))
      , num_elements(other.num_elements)
    {
      if (table.empty()) table.resize(INIT_CAPACITY);
      other.num_elements = 0;
    }

    RobinHoodMap & operator=(const RobinHoodMap & other) = default;
    RobinHoodMap & operator=(RobinHoodMap && other) noexcept {
      if (this == &other) return *this;
      table        = std::move(other.table);
      num_elements = other.num_elements;
      if (table.empty()) table.resize(INIT_CAPACITY);
      other.num_elements = 0;
      return *this;
    }

    ~RobinHoodMap() = default;

    // === ITERATOR TYPES ===

    template <bool IS_CONST>
    class iterator_base {
    private:
      friend class RobinHoodMap;

      // Build types based on if this is a const iterator.
      using MapType   = std::conditional_t<IS_CONST, const RobinHoodMap, RobinHoodMap>;
      using EntryType = std::conditional_t<IS_CONST, const Entry, Entry>;
      using MappedRef = std::conditional_t<IS_CONST, const T, T>;
  
      MapType * map_ptr = nullptr;  // Map that owns this iterator
      size_t index      = 0;        // Current index in the table

      // Proxy type used as iterator::reference; works with structured bindings.
      struct Ref {
        const Key & first;
        MappedRef & second;

        // Implicit conversion to std::pair for algorithms that expect value_type.
        operator std::pair<const Key, T>() const { return { first, second }; }
      };

      bool MakeValid() {
        if (!map_ptr) return false; // Cannot make valid.
        const size_t table_size = map_ptr->table.size();
        while (index < table_size && !map_ptr->table[index]) {
          ++index;
        }
        return true;
      }

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type        = std::pair<const Key, T>;
      using difference_type   = std::ptrdiff_t;
      using reference         = Ref;
      using pointer           = void;  // no real pointer type

      iterator_base() = default;
      iterator_base(MapType * map_ptr, size_t index=0) : map_ptr(map_ptr), index(index) {
        MakeValid();
      }
      iterator_base(const iterator_base &) = default;
      iterator_base & operator=(const iterator_base &) = default;
      ~iterator_base() = default;

      bool IsValid() const {
        return map_ptr &&                        // Does map exist?
               index < map_ptr->table.size() &&  // With the needed index?
               map_ptr->table[index].occupied;   // And that index is occupied?
      }

      reference operator*() const {
        emp_assert(IsValid());
        auto & entry = map_ptr->table[index];
        return Ref{ entry.key, entry.value };
      }

      // If you really want ->, you can return a by-value proxy; often not needed.
      // pointer operator->() const = delete;

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
    using key_type        = Key;
    using mapped_type     = T;
    using value_type      = std::pair<const Key, T>;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

    [[nodiscard]] size_t size() const { return num_elements; }

    [[nodiscard]] size_t bucket_count() const { return table.size(); }

    [[nodiscard]] size_t max_load() const { return table.size() / LOAD_FACTOR; }

    [[nodiscard]] bool empty() const { return num_elements == 0; }

    [[nodiscard]] bool contains(const Key & key) const { return FindPtr(key) != nullptr; }

    [[nodiscard]] iterator begin() { return iterator(this, 0); }
    [[nodiscard]] iterator end() { return iterator(this, table.size()); }
    [[nodiscard]] const_iterator begin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator end() const { return const_iterator(this, table.size()); }
    [[nodiscard]] const_iterator cbegin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(this, table.size()); }

    void clear() {
      for (auto & entry : table) entry.occupied = false;
      num_elements = 0;
    }

    // Make sure that we can store at least n elements without additional allocations.
    void reserve(size_t n) {
      // If we already have enough capacity, do nothing.
      if (n <= max_load()) return;

      // Grow with the same rule used in Insert
      size_t new_max_load = std::max(table.size(), INIT_CAPACITY);
      while (new_max_load < n) new_max_load *= GROW_FACTOR;

      Rehash(new_max_load * LOAD_FACTOR);
    }

    std::pair<iterator,bool> insert(const std::pair<const Key, T> & in) {
      return Insert(in.first, in.second);
    }

    /// Insert with "unique key" semantics.
    /// @return pair of iterator to key and bool to indicate if a new element was inserted.
    std::pair<iterator,bool> Insert(const Key & key, const T & value) {
      // Test if we need to grow the table...
      if (num_elements >= max_load()) { Rehash(table.size() * GROW_FACTOR); }

      // Search for an existing key. Return false if we find one; end loop if there is not one.
      SearchPos test_pos{MakeSearchPos(key)};
      while (table[test_pos]) {        
        if (TestAt(test_pos, key)) {
          return { iterator{this, test_pos}, false };  // Key already in table.
        }
        if (test_pos.dist > CalcOffset(test_pos)) break; // Current is "closer to home", start Robin Hood swap-in.

        test_pos.Next(table.size()); // Keep searching at the next position.
      }

      // Current key not in table; perform standard Robin Hood insertion from found position.
      Entry new_entry{key, value, test_pos.hash, true};
      size_t found_pos = test_pos;

      // Search for an empty position, juggling entries as we go.
      while (table[test_pos]) {
        // Otherwise bump entry closer to home.
        const size_t existing_dist = CalcOffset(test_pos);
        if (existing_dist < test_pos.dist) {
          std::swap(table[test_pos], new_entry);
          test_pos.dist = existing_dist;
        }

        test_pos.Next(table.size());  // Move on to the next table position.
      }

      table[test_pos] = std::move(new_entry);
      ++num_elements;
      return { iterator{this, found_pos}, true };
    }

    [[nodiscard]] const T * FindPtr(const Key & key) const { return FindPtr_impl(key); }

    [[nodiscard]] T * FindPtr(const Key & key) { return const_cast<T*>(FindPtr_impl(key)); }

    T & operator[](const Key & key) {
      T * val = FindPtr(key);
      if (val) { return *val; }

      Insert(key, T{});
      return *FindPtr(key);
      // auto it = Insert(key, T{}).first;
      // return (*it).second;
    }

    bool erase(const Key & key) {
      if (table.empty()) { return false; }  // Nothing to delete.

      SearchPos test_pos{MakeSearchPos(key)};
      while (table[test_pos] && CalcOffset(test_pos) >= test_pos.dist) {
        if (TestAt(test_pos, key)) {  // If we found key, begin deletion and backshift
          EraseAt(test_pos);
          return true;
        }

        test_pos.Next(table.size());
      }

      return false;  // Couldn't find in table.
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
