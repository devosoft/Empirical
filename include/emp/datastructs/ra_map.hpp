/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file ra_map.hpp
 *  @brief This file defines a Random Access Map template.
 *  @note Status: ALPHA
 * 
 *  A random access map allows for simple traversal by index and a guarantee that a value at a
 *  given index will always be at that index unless any map element is deleted.  This allows
 *  storage of indices for maps with a fixed layout, resulting in easy access.
 */

#ifndef EMP_DATASTRUCTS_RA_MAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_RA_MAP_HPP_INCLUDE

#include <map>

#include "../base/unordered_map.hpp"
#include "../base/vector.hpp"
#include "../math/constants.hpp"

namespace emp {

  /// This class uses a combination of a hashtable (std::unordered_map) and emp::vector to
  /// lookup insert, lookup, and delete values in constant time, while still being able to
  /// step through all values (albeit in an arbitrary order).
  ///
  /// @note The arbitrary order of values may change if any values are deleted.

  template <typename KEY_T,
            typename T,
            typename Hash = std::hash<KEY_T>,
            typename KeyEqual = std::equal_to<KEY_T>,
            typename Allocator = std::allocator< std::pair<const KEY_T, T> >
           >
  class ra_map {
  public:
    using key_type = KEY_T;
    using mapped_type = T;
    using value_type = std::pair<const KEY_T, T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

  private:
    emp::unordered_map<KEY_T,size_t,Hash,KeyEqual> id_map;  ///< Map to find keys in vector.
    emp::vector<value_type> vals;                           ///< Vector of all values.

    using this_t = ra_map<KEY_T,T,Hash,KeyEqual,Allocator>;
  public:
    ra_map() = default;
    ra_map(const ra_map &) = default;
    ra_map(ra_map &&) = default;
    this_t & operator=(const ra_map &) = default;
    this_t & operator=(ra_map &&) = default;

    // -- Iterators --
    auto begin() { return vals.begin(); }
    auto cbegin() const { return vals.cbegin(); }
    auto end() { return vals.end(); }
    auto cend() const { return vals.cend(); }

    // -- Capacity --
    size_t size() const { return vals.size(); }           ///< Number of entries in map.
    bool empty() const { return size() == 0; }            ///< Are there NO values in map?
    size_t max_size() const { return id_map.max_size(); } ///< Max system limit on size.

    // -- Modifiers --
    void clear() { id_map.clear(); vals.resize(0); }      ///< Remove all values from container.

    /// Insert a new value into container by copy; return position.
    size_t insert(const value_type & v) {
      auto pos_it = id_map.find(v.first);
      if (pos_it != id_map.end()) return pos_it->second; // Already in map.
      const size_t pos = vals.size();
      id_map[v.first] = pos;
      vals.emplace_back(v);
      return pos;
    }

    /// Insert a new value into container by move; return position.
    size_t insert(value_type && v) {
      auto pos_it = id_map.find(v.first);
      if (pos_it != id_map.end()) return pos_it->second; // Already in map.
      const size_t pos = vals.size();
      id_map[v.first] = pos;
      vals.emplace_back(std::move(v));
      return pos;
    }

    /// Construct a new value in place in a container container; return position.
    template <typename... Ts>
    size_t emplace(Ts &&... args) {
      const size_t new_pos = vals.size();
      vals.emplace_back(std::forward<Ts>(args)...);
      auto old_pos_it = id_map.find(vals.back().first);
      if (old_pos_it != id_map.end()) {
        vals.resize(vals.size()-1);     // Destroy newly created instance.
        return old_pos_it->second;      // Return old position in map.
      }
      id_map[vals.back().first] = new_pos;  // Save new position for later lookup.
      return new_pos;                   // And return it.
    }

    /// Erase a specific value from the container.
    bool erase(const KEY_T & key) {
      if (!count(key)) return false;   // Not in map.

      // Find out where key is in id_map and clear it.
      const size_t pos = id_map[key];
      id_map.erase(key);

      // Move the former last value to the now-empty spot.
      const size_t last_pos = vals.size() - 1;
      if (pos != last_pos) {
        const_cast<KEY_T&>(vals[pos].first) = vals[last_pos].first;
        vals[pos].second = vals[last_pos].second;
        id_map[vals[pos].first] = pos;
      }
      vals.resize(last_pos);
      return true;
    }

    
    size_t count(const KEY_T & key) const { return id_map.count(key); } /// Is value included? (0 or 1).

    /// Index into the ra_map by key.
    T & operator[](key_type key) {
      auto key_it = id_map.find(key);
      if (key_it == id_map.end()) {
        return NewEntry(key);
      }
      return vals[key_it->second].second;
    }

    // --- Empirical only commands ---

    T & NewEntry(key_type key) {
      emp_assert(id_map.find(key) == id_map.end(), "ra_map::NewEntry must be an unused key!", key);
      const size_t pos = vals.size();
      id_map[key] = pos;
      vals.emplace_back();
      return vals.back().second;
    }

    bool Has(key_type key) const { return id_map.find(key) != id_map.end(); }

    size_t GetID(key_type key) {
      auto key_it = id_map.find(key);
      return (key_it == id_map.end()) ? emp::MAX_SIZE_T : key_it->second;
    }

    key_type & KeyAtID(size_t id) { return vals[id]->first; }

    T & AtID(size_t id) { return vals[id]->second; }
    const T & AtID(size_t id) const { return vals[id]->second; }
  };

}

#endif // #ifndef EMP_DATASTRUCTS_RA_SET_HPP_INCLUDE
