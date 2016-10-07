//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A Cache works similar to an unordered_map, but all lookups come with a function to generate
//  the result if the lookup fails.

#ifndef EMP_CACHE_H
#define EMP_CACHE_H

#include <unordered_map>

namespace emp {

  template <class Key,                                        // Cache::key_type
	    class T,                                                // Cache::mapped_type
	    class Hash = std::hash<Key>,                            // Cache::hasher
	    class Pred = std::equal_to<Key>,                        // Cache::key_equal
	    class Alloc = std::allocator< std::pair<const Key,T> >  // Cache::allocator_type
	   >
  class Cache {
  private:
    std::unordered_map<Key, T, Hash, Pred, Alloc> cache_map;

  public:
    using key_type = Key;
    using mapped_type = T;
    using hasher= Hash;
    using key_equal = Pred;
    using allocator_type = Alloc;

    size_t size() const { return cache_map.size(); }

    bool Has(const Key & k) const { return cache_map.find(k) != cache_map.end(); }
    void Clear() { cache_map.clear(); }
    void Erase(const Key & k) { cache_map.erase(k); }

    T Get(Key k, const std::function<T(Key k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

    // A version of Get that allows calls with const references instead of pass-by-value.
    const T & GetRef(const Key & k, const std::function<T(const Key & k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

  };

}

#endif
