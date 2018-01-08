/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  Cache.h
 *  @brief similar to an std::unordered_map, but all lookups come with a function to generate the result should the lookup fail.
 *  @note Status: BETA
 */

#ifndef EMP_CACHE_H
#define EMP_CACHE_H

#include <functional>
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
    Cache() : cache_map() { }
    Cache(const Cache &) = default;
    Cache(Cache &&) = default;
    Cache & operator=(const Cache &) = default;
    Cache & operator=(Cache &&) = default;

    using key_type = Key;          ///< Type we are using to look up values.
    using mapped_type = T;         ///< Contents of the value we look up.
    using hasher = Hash;           ///< Hash method to use.
    using key_equal = Pred;        ///< Function to test if two values are identical.
    using allocator_type = Alloc;  ///< Function to allocate new space.

    /// How many entries are stored in the cache?
    size_t size() const { return cache_map.size(); }

    /// Determine if a specific key is already in the cache.
    bool Has(const Key & k) const { return cache_map.find(k) != cache_map.end(); }

    /// Erase contents of cache.
    void Clear() { cache_map.clear(); }

    /// Erase a specific entry from cache.
    void Erase(const Key & k) { cache_map.erase(k); }

    /// Lookup a specific key; provide a function to use if value is not in cahce.
    T Get(Key k, const std::function<T(Key k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

    /// A version of Get that allows calls with const references instead of pass-by-value.
    const T & GetRef(const Key & k, const std::function<T(const Key & k)> & calc_fun) {
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(k, calc_fun(k)).first->second;
    }

  };

}

#endif
