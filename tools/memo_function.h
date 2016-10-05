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
#include "tuple_utils.h"

namespace emp {

  template <class T> class memo_function;    // Not defined.

  template <class R, class... ARGS>
  class memo_function<R(ARGS...)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using fun_t = std::function<R(ARGS...)>;
    using hash_t = emp::TupleHash<ARGS...>;
    using this_t = memo_function<R(ARGS...)>;
    using tuple_t = std::tuple<std::decay_t<ARGS>...>;

  private:
    std::unordered_map<tuple_t, return_t, hash_t> cache_map;
    fun_t fun;

  public:
    template <typename... Ts>
    memo_function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { cache_map.clear(); fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { cache_map.clear(); fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { cache_map.clear(); fun = std::forward<T>(arg); return *this; }

    size_t size() const { return cache_map.size(); }

    inline static size_t Hash(const ARGS &... k) { return CombineHash(k...); }
    bool Has(const ARGS &... k) const { return cache_map.find(std::make_tuple(k...)) != cache_map.end(); }
    void Clear() { cache_map.clear(); }
    void Erase(const ARGS &... k) { cache_map.erase(std::make_tuple(k...)); }

    return_t operator()(ARGS... k) {
      emp_assert(fun); // Function must be specified with Get() -or- already set.
      auto cache_it = cache_map.find(std::make_tuple(k...));
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(std::make_tuple(k...), fun(std::forward<ARGS>(k)...)).first->second;
    }

    operator bool() { return (bool) fun; }
  };

}

#endif
