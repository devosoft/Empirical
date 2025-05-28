/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief Robin Hood hashing - minimize how "wrong" a position can be in a hash table.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_INCLUDE

#include "../base/vector.hpp"

namespace emp {
  template <typename Key, typename T>
  class RobinHoodMap {
  private:
    struct Entry {
      Key key;
      T value;
      size_t hash = 0;        // Store hash to avoid recomputing
      bool occupied = false;  // @CAO Optimize away later
    };

    static constexpr double MAX_LOAD_FACTOR = 0.8;
    static constexpr size_t INIT_CAPACITY = 16;
    static constexpr double GROW_FACTOR = 2.0;
    static constexpr size_t GROW_OFFSET = 0;

    emp::vector<Entry> table{INIT_CAPACITY};
    size_t num_elements = 0;


    // === HELPER FUNCTIONS ===

    // Calculate how far off a current entry is from its ideal position.
    [[nodiscard]] size_t CalcDist(size_t pos) const {
      emp_assert(table[pos].occupied, pos);
      return (pos + capacity() - (table[pos].hash % capacity())) % capacity();
    }

    void Rehash(size_t new_capacity) {
      emp::vector<Entry> old_table = std::move(table);
      table = emp::vector<Entry>(new_capacity);
      num_elements = 0;

      // Move all elements into the new hash table.
      for (const Entry & entry : old_table) {
        if (entry.occupied) {
          Insert(entry.key, entry.value);
        }
      }
    }

    [[nodiscard]] const T * FindPtr_impl(const Key & key) const {
      size_t hash_value = std::hash<Key>{}(key);
      size_t pos = hash_value % capacity();
      size_t dist = 0;

      while (true) {
        if (!table[pos].occupied) { return nullptr; } // Not found.

        if (table[pos].hash == hash_value && table[pos].key == key) { return &table[pos].value; }

        size_t existing_dist = CalcDist(pos);
  
        // If existing_dist has gotten too high, we would have swapped; stop here.
        if (existing_dist < dist) return nullptr; // Early exit: key not in table

        pos = (pos + 1) % capacity();
        ++dist;
      }
    }

  public:
    RobinHoodMap() = default;
    RobinHoodMap(RobinHoodMap &&) = default;
    RobinHoodMap & operator=(RobinHoodMap &&) = default;
    ~RobinHoodMap() { }

    [[nodiscard]] size_t size() const { return num_elements; }
    [[nodiscard]] size_t capacity() const { return table.size(); }
    [[nodiscard]] bool empty() const { return num_elements == 0; }

    void Insert(const Key & key, const T & value) {
      // Test if we need to grow the table...
      if (num_elements / static_cast<double>(capacity()) >= MAX_LOAD_FACTOR) {
        Rehash(capacity() * GROW_FACTOR + GROW_OFFSET);
      }

      const size_t hash_value = std::hash<Key>{}(key);
      size_t pos = hash_value % capacity();
      size_t dist = 0;

      Entry new_entry{key, value, hash_value, true};

      while (true) {
        if (!table[pos].occupied) {
          table[pos] = new_entry;
          ++num_elements;
          return;
        }

        const size_t existing_dist = CalcDist(pos);
        if (existing_dist < dist) {
          std::swap(table[pos], new_entry);
          dist = existing_dist;
        }

        pos = (pos + 1) % capacity();
        ++dist;
      }
    }

    [[nodiscard]] const T * FindPtr(const Key & key) const { return FindPtr_impl(key); }

    [[nodiscard]] T * FindPtr(const Key & key) { return const_cast<T*>(FindPtr_impl(key)); }


    T & operator[](const Key & key) {
      T * val = FindPtr(key);
      if (val) return *val;

      Insert(key, T{});
      return *FindPtr(key);
    }

    bool erase(const Key& key) {
      if (table.empty()) { return false; } // Nothing to delete.

      const size_t hash_value = std::hash<Key>{}(key);
      size_t pos = hash_value % capacity();
      size_t dist = 0;

      while (true) {
        if (!table[pos].occupied) { return false; } // Empty pos -> not in table.

        if (table[pos].hash == hash_value && table[pos].key == key) {
          // Found key; begin deletion and backshift
          size_t next = (pos + 1) % capacity();
          while (table[next].occupied) {
            const size_t probe_dist = CalcDist(next);

            if (probe_dist == 0) break;  // Entry is already in ideal spot

            table[pos] = std::move(table[next]);
            pos  = next;
            next = (next + 1) % capacity();
          }

          // Clear final position
          table[pos] = Entry{};
          --num_elements;
          return true;
        }

        size_t existing_dist = CalcDist(pos);
        if (existing_dist < dist) { return false; }

        pos = (pos + 1) % capacity();
        ++dist;
      }
    }
  };

} // namespace emp

#endif // EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_INCLUDE