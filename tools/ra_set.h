//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a Random Access Set template.
//
//  This class uses a combination of a hashtable (std::unordered_map) and emp::vector to
//  lookup insert, lookup, and delete values in constant time, while still being able to
//  step through values in and arbitrary order.
//
//  NOTE: the order may change if values are deleted.

#ifndef EMP_RA_SET_H
#define EMP_RA_SET_H

#include <map>

#include "../base/vector.h"

namespace emp {

  template <typename T>
  class ra_set {
  private:
    std::map<T,size_t> id_map;
    emp::vector<T> vals;

  public:
    ra_set() = default;
    ra_set(const ra_set &) = default;
    ra_set(ra_set &&) = default;
    ra_set<T> & operator=(const ra_set &) = default;
    ra_set<T> & operator=(ra_set &&) = default;
    
    using value_type = T;

    bool empty() const { return id_map.size() == 0; }
    size_t size() const { return id_map.size(); }

    const T & operator[](size_t pos) const { return vals[pos]; }
    
    void clear() { id_map.clear(); vals.resize(0); }
    void insert(const T & v) {
      if (count(v)) return; // Already in set.
      const size_t pos = vals.size();
      vals.push_back(v);
      id_map[v] = pos;
    }
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

    size_t count(const T & v) const { return id_map.count(v); }
  };

}

#endif
