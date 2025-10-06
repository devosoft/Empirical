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
      size_t hash   = 0;      // Store hash to avoid recomputing
      bool occupied = false;  // @CAO Optimize away later

      operator bool() const { return occupied; }
    };

    static constexpr double MAX_LOAD_FACTOR = 0.8;
    static constexpr size_t INIT_CAPACITY   = 15;
    static constexpr size_t GROW_FACTOR     = 2;
    static constexpr size_t GROW_OFFSET     = 1;

    emp::vector<Entry> table{INIT_CAPACITY};
    size_t num_elements = 0;

    // === HELPER FUNCTIONS ===

    static size_t ImproveHash(size_t hash_value) {
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
    [[nodiscard]] size_t CalcDist(size_t pos) const {
      emp_assert(table[pos].occupied, pos);
      const size_t ideal_pos = table[pos].hash % capacity();
      return (pos + capacity() - ideal_pos) % capacity();
    }

    void Rehash(size_t new_capacity) {
      emp::vector<Entry> old_table = std::move(table);
      table = emp::vector<Entry>(new_capacity);
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
        pos = (pos + 1) % table_size;
        ++dist;
      }
      operator size_t() const { return pos; }
    };

    SearchPos MakeSearchPos(const Key & key) const {
      const size_t hash = CalcHash(key);
      return SearchPos{hash, hash % capacity(), 0};
    }

    bool TestAt(const SearchPos & test_pos, const Key & key) const {
      return table[test_pos.pos].hash == test_pos.hash && table[test_pos.pos].key == key;
    }

    [[nodiscard]] const T * FindPtr_impl(const Key & key) const {
      SearchPos test_pos{MakeSearchPos(key)};

      while (table[test_pos] && test_pos.dist <= CalcDist(test_pos)) {
        if (TestAt(test_pos, key)) { return &table[test_pos].value; } // Found!
        test_pos.Next(capacity()); // Try the next position.
      }
      return nullptr; // Not found!
   }

    // Erase the element at a specified position in the table and do backshift
    void EraseAt(size_t pos) {
      size_t next = (pos + 1) % capacity();
      while (table[next] && CalcDist(next) != 0) { // Test if next entry should move up.
        table[pos] = std::move(table[next]);
        pos        = next;
        next       = (next + 1) % capacity();
      }

      // Clear final position
      table[pos].occupied = false;
      --num_elements;
    }

  public:
    // Constructors and destructor are implicitly created.

    [[nodiscard]] size_t size() const { return num_elements; }

    [[nodiscard]] size_t capacity() const { return table.size(); }

    [[nodiscard]] bool empty() const { return num_elements == 0; }

    [[nodiscard]] bool contains(const Key & key) const { return FindPtr(key) != nullptr; }

    void clear() {
      for (auto & entry : table) entry.occupied = false;
      num_elements = 0;
    }

    // Make sure that we can store at least n elements without additional allocations.
    void reserve(size_t n) {
      // If we already have enough capacity, do nothing.
      if (n <= static_cast<size_t>(capacity() * MAX_LOAD_FACTOR)) return;

      // Grow with the same rule used in Insert
      size_t new_cap = table.size();
      while (static_cast<double>(new_cap) * MAX_LOAD_FACTOR < n) {
        new_cap = static_cast<size_t>(new_cap * GROW_FACTOR + GROW_OFFSET);
      }

      Rehash(new_cap);
    }

    // std::map-like insert interface (minimal version).
    // std::map returns pair<iterator,bool>; we just return bool for now.
    bool insert(const std::pair<Key, T> & in) {
      return Insert(in.first, in.second);
    }

    /// Insert with "unique key" semantics.
    /// @return true if a new element was inserted, false if key was already present.
    bool Insert(const Key & key, const T & value) {
      // Test if we need to grow the table...
      if (num_elements >= MAX_LOAD_FACTOR * capacity()) {
        Rehash(capacity() * GROW_FACTOR + GROW_OFFSET);
      }

      // Search for an existing key. Return false if we find one; end loop if there is not one.
      SearchPos test_pos{MakeSearchPos(key)};
      while (table[test_pos]) {        
        if (TestAt(test_pos, key)) return false;       // Key is already in the table.
        if (test_pos.dist > CalcDist(test_pos)) break; // Current is "closer to home", start Robin Hood swap-in.

        test_pos.Next(capacity()); // Keep searching at the next position.
      }

      // Current key not in table; perform standard Robin Hood insertion from found position.
      Entry new_entry{key, value, test_pos.hash, true};

      // Search for an empty position, juggling entries as we go.
      while (table[test_pos]) {
        // Otherwise bump entry closer to home.
        const size_t existing_dist = CalcDist(test_pos);
        if (existing_dist < test_pos.dist) {
          std::swap(table[test_pos], new_entry);
          test_pos.dist = existing_dist;
        }

        test_pos.Next(capacity());  // Move on to the next table position.
      }

      table[test_pos] = std::move(new_entry);
      ++num_elements;
      return true;
    }

    [[nodiscard]] const T * FindPtr(const Key & key) const { return FindPtr_impl(key); }

    [[nodiscard]] T * FindPtr(const Key & key) { return const_cast<T*>(FindPtr_impl(key)); }

    T & operator[](const Key & key) {
      T * val = FindPtr(key);
      if (val) { return *val; }

      Insert(key, T{});
      return *FindPtr(key);
    }

    bool erase(const Key & key) {
      if (table.empty()) { return false; }  // Nothing to delete.

      SearchPos test_pos{MakeSearchPos(key)};
      while (table[test_pos] && CalcDist(test_pos) >= test_pos.dist) {
        if (TestAt(test_pos, key)) {  // If we found key, begin deletion and backshift
          EraseAt(test_pos);
          return true;
        }

        test_pos.Next(capacity());
      }

      return false;  // Couldn't find in table.
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
