/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2019
 *
 *  @file ra_set.hpp
 *  @brief This file defines a Random Access Set template.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATASTRUCTS_RA_SET_HPP_INCLUDE
#define EMP_DATASTRUCTS_RA_SET_HPP_INCLUDE

#include <map>

#include "../base/map.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// This class uses a combination of a hashtable (std::unordered_map) and emp::vector to
  /// lookup insert, lookup, and delete values in constant time, while still being able to
  /// step through all values (albeit in an arbitrary order).
  ///
  /// @note The arbitrary order of values may change if any values are deleted.

  template <typename T>
  class ra_set {
  private:
    emp::map<T,size_t> id_map;  ///< A map of where to find values in vector.
    emp::vector<T> vals;        ///< A vector of all values contained.

  public:
    ra_set() = default;
    ra_set(const ra_set &) = default;
    ra_set(ra_set &&) = default;
    ra_set<T> & operator=(const ra_set &) = default;
    ra_set<T> & operator=(ra_set &&) = default;

    using value_type = T;

    /// Are there any values in this ra_set?
    bool empty() const { return id_map.size() == 0; }

    /// How many elements are in this set?
    size_t size() const { return id_map.size(); }

    /// Index into the ra_set, similar to a vector.
    const T & operator[](size_t pos) const { return vals[pos]; }

    /// Remove all values from this container.
    void clear() { id_map.clear(); vals.resize(0); }

    /// Insert a new value into container.
    void insert(const T & v) {
      if (count(v)) return; // Already in set.
      const size_t pos = vals.size();
      vals.push_back(v);
      id_map[v] = pos;
    }

    /// Erase a specific value from the container.
    bool erase(const T & v) {
      if (!count(v)) return false;   // Not in set.

      // Find out where v is in id_map and clear it.
      const size_t pos = id_map[v];
      id_map.erase(v);

      // Move the former last value to the now-empty spot.
      const size_t last_pos = vals.size() - 1;
      if (pos != last_pos) {
        vals[pos] = vals[last_pos];
        id_map[vals[pos]] = pos;
      }
      vals.resize(last_pos);
      return true;
    }

    /// Count the number of times a particular value in in the container (0 or 1).
    size_t count(const T & v) const { return id_map.count(v); }
  };

}

#endif // #ifndef EMP_DATASTRUCTS_RA_SET_HPP_INCLUDE
