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

  template <class T> class memo_function;    // Not defined.

  template <class R, class... ARGS>
  class memo_function<R(ARGS...)> {
  public:
    using key_type = std::tuple<ARGS...>;
    using mapped_type = R;
    using fun_t = std::function<R(ARGS...)>;

  private:
    std::unordered_map<key_type, R> cache_map;

  public:
    size_t size() const { return cache_map.size(); }

    bool Has(const ARGS &... k) const {
      return cache_map.find(key_type(k...)) != cache_map.end();
    }
    void Clear() { cache_map.clear(); }
    void Erase(const ARGS &... k) { cache_map.erase(key_type(k...)); }

    R Get(ARGS... k, const fun_t & calc_fun) {
      auto cache_it = cache_map.find(key_type(k...));
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(key_type(k...), calc_fun(k...)).first->second;
    }

  };

}

#endif
