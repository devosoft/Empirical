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

#include "assert.h"
#include "meta.h"

namespace emp {

  template <class T> class memo_function;    // Not defined.

  template <class R, class... ARGS>
  class memo_function<R(ARGS...)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using fun_t = std::function<R(ARGS...)>;
    using this_t = memo_function<R(ARGS...)>;

  private:
    std::unordered_map<size_t, return_t> cache_map;
    fun_t fun;

  public:
    template <typename... Ts>
    memo_function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { cache_map.clear(); fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { cache_map.clear(); fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { fun = std::forward<T>(arg); return *this; }

    size_t size() const { return cache_map.size(); }

    inline static size_t Hash(const ARGS &... k) { return CombineHash(k...); }
    bool Has(const ARGS &... k) const { return cache_map.find(Hash(k...)) != cache_map.end(); }
    void Clear() { cache_map.clear(); }
    void Erase(const ARGS &... k) { cache_map.erase(Hash(k...)); }

    return_t operator()(ARGS... k) {
      emp_assert(fun); // Function must be specified with Get() -or- already set.
      auto cache_it = cache_map.find(Hash(k...));
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(Hash(k...), fun(std::forward<ARGS>(k)...)).first->second;
    }

    operator bool() { return (bool) fun; }
  };

}

#endif
