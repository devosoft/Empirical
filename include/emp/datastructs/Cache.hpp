/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  Cache.hpp
 *  @brief similar to an std::unordered_map, but all lookups come with a function to generate the result should the lookup fail.
 *  @note Status: BETA
 */

#ifndef EMP_CACHE_H
#define EMP_CACHE_H

#include <functional>
#include <unordered_map>

namespace emp {

  template <class KEY,                                        // Cache::key_type
	    class T,                                                // Cache::mapped_type
	    class HASH = std::hash<KEY>,                            // Cache::hasher
	    class PRED = std::equal_to<KEY>,                        // Cache::key_equal
	    class ALLOC = std::allocator< std::pair<const KEY,T> >  // Cache::allocator_type
	   >
  class Cache {
  private:
    std::unordered_map<KEY, T, HASH, PRED, ALLOC> cache_map;

  public:
    Cache() : cache_map() { }
    Cache(const Cache &) = default;
    Cache(Cache &&) = default;
    Cache & operator=(const Cache &) = default;
    Cache & operator=(Cache &&) = default;

    using key_type = KEY;          ///< Type we are using to look up values.
    using mapped_type = T;         ///< Contents of the value we look up.
    using hasher = HASH;           ///< Hash method to use.
    using key_equal = PRED;        ///< Function to test if two values are identical.
    using allocator_type = ALLOC;  ///< Function to allocate new space.

    /// How many entries are stored in the cache?
    size_t size() const { return cache_map.size(); }

    /// Determine if a specific key is already in the cache.
    bool Has(const KEY & k) const { return cache_map.find(k) != cache_map.end(); }

    /// Erase contents of cache.
    void Clear() { cache_map.clear(); }

    /// Erase a specific entry from cache.
    void Erase(const KEY & k) { cache_map.erase(k); }

    /// Lookup a specific key; provide a function to use if value is not in cache.
    T Get(KEY k, const std::function<T(KEY k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

    /// A version of Get that allows calls with const references instead of pass-by-value.
    const T & GetRef(const KEY & k, const std::function<T(const KEY & k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

  };

}

#endif
