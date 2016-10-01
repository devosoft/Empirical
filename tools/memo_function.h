//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A memo_function works identicaly to std::function, but memorizes prior results (caches them)
//  so that the function doesn't have to process those results again.

#ifndef EMP_MEMO_FUNCTIONS_H
#define EMP_MEMO_FUNCTIONS_H

#include <unordered_map>

namespace emp {

  template <class R,                                          // memo_function::mapped_type (was T)
	    class... ARGS,                                          // memo_function::key_type (was Key)
	    class HASH=std::hash<std::tuple<ARGS...>>,              // memo_function::hasher
	    class PRED=std::equal_to<std::tuple<ARGS...>>,                      // memo_function::key_equal
	    class ALLOC=std::allocator<std::pair<const std::tuple<ARGS...>,R>>  // memo_function::allocator_type
	   >
  class memo_function<R(ARGS...)> {
  public:
    using key_type = std::tuple<ARGS...>;
    using mapped_type = R;
    using hasher = HASH;
    using key_equal = PRED;
    using allocator_type = ALLOC;

  private:
    std::unordered_map<key_type, mapped_type, HASH, PRED, ALLOC> cache_map;

  public:
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
