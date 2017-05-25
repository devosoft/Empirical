//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A memo_function works identicaly to std::function, but memorizes prior results (caches them)
//  so that the function doesn't have to process those results again.
//  Status: BETA

#ifndef EMP_MEMO_FUNCTIONS_H
#define EMP_MEMO_FUNCTIONS_H

#include <unordered_map>

#include "../base/assert.h"
#include "../meta/meta.h"

#include "tuple_utils.h"

namespace emp {

  template <class T> class memo_function;    // Not defined.

  // Single argument functions don't need a tuple...
  template <class R, class ARG>
  class memo_function<R(ARG)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using index_t = std::decay_t<ARG>;
    using fun_t = std::function<R(ARG)>;
    using this_t = memo_function<R(ARG)>;

  private:
    mutable std::unordered_map<index_t, return_t> cache_map;
    fun_t fun;

  public:
    template <typename T>
    memo_function(T && fun_info) : fun(std::forward<T>(fun_info)) { ; }
    memo_function(const this_t &) = default;
    memo_function(this_t &&) = default;
    memo_function() { ; }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { cache_map.clear(); fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { cache_map.clear(); fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { cache_map.clear(); fun = std::forward<T>(arg); return *this; }

    size_t size() const { return cache_map.size(); }

    bool Has(const ARG & k) const { return cache_map.find(k) != cache_map.end(); }
    void Clear() { cache_map.clear(); }
    void Erase(const ARG & k) { cache_map.erase(k); }

    template <class KEY>
    return_t operator()(KEY && k) const {
      emp_assert(fun);
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      const return_t result = fun(k);
      return cache_map.emplace(std::forward<KEY>(k), result).first->second;
    }

    operator bool() const { return (bool) fun; }

    // A memo_function can be converted to a regular std::function for function calls.
    operator std::function<R(ARG)>() {
      return [this](const ARG & arg){ return operator()(arg); };
    }
    std::function<R(ARG)> to_function() {
      return [this](const ARG & arg){ return operator()(arg); };
    }
  };

  // Specialization for when we have more than one argument...  we need to convert inputs
  // to a tuple to make this work.
  template <class R, class A1, class A2, class... EXTRA>
  class memo_function<R(A1,A2,EXTRA...)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using fun_t = std::function<R(A1,A2,EXTRA...)>;
    using hash_t = emp::TupleHash<A1,A2,EXTRA...>;
    using this_t = memo_function<R(A1,A2,EXTRA...)>;
    using tuple_t = std::tuple<std::decay_t<A1>,std::decay_t<A2>,std::decay_t<EXTRA>...>;

  private:
    std::unordered_map<tuple_t, return_t, hash_t> cache_map;
    fun_t fun;

  public:
    template <typename... Ts>
    memo_function(Ts &&... args) : fun(std::forward<Ts>(args)...) { ; }
    memo_function(const memo_function &) = default;
    memo_function(memo_function &&) = default;

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { cache_map.clear(); fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { cache_map.clear(); fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { cache_map.clear(); fun = std::forward<T>(arg); return *this; }

    size_t size() const { return cache_map.size(); }

    inline static size_t Hash(const A1 & k1, const A2 & k2, const EXTRA &... k_extra) {
      return CombineHash(k1,k2,k_extra...);
    }
    bool Has(const A1 & k1, const A2 & k2, const EXTRA &... k_extra) const {
      return cache_map.find(std::make_tuple(k1,k2,k_extra...)) != cache_map.end();
    }
    void Clear() { cache_map.clear(); }
    void Erase(const A1 & k1, const A2 & k2, const EXTRA &... k_extra) {
      cache_map.erase(std::make_tuple(k1,k2,k_extra...));
    }

    return_t operator()(const A1 & k1, const A2 & k2, const EXTRA &... k_extra) {
      emp_assert(fun); // Function must be specified with Get() -or- already set.
      auto cache_it = cache_map.find(std::make_tuple(k1,k2,k_extra...));
      if (cache_it != cache_map.end()) return cache_it->second;
      return cache_map.emplace(std::make_tuple(k1,k2,k_extra...),
                               fun(k1, k2, k_extra...)).first->second;
    }

    operator bool() const { return (bool) fun; }

    // A memo_function can be converted to a regular std::function for function calls.
    operator std::function<R(A1,A2,EXTRA...)>() const {
      return [this](A1 k1, A2 k2, EXTRA... k_extra) {
         return operator()(k1, k2, k_extra...);
      };
    }
    std::function<R(A1,A2,EXTRA...)> to_function() const {
      return [this](A1 k1, A2 k2, EXTRA... k_extra) {
         return operator()(k1, k2, k_extra...);
      };
    }
  };

  // Single argument functions don't need a tuple...
  template <class R>
  class memo_function<R()> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using index_t = void;
    using fun_t = std::function<R()>;
    using this_t = memo_function<R()>;

  private:
    return_t cached_value;
    bool has_cache;
    fun_t fun;

  public:
    template <typename T>
    memo_function(T && fun_info) : has_cache(false), fun(std::forward<T>(fun_info)) { ; }
    memo_function(const this_t &) = default;
    memo_function(this_t &&) = default;
    memo_function() : has_cache(false) { ; }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { has_cache=false; fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { has_cache=false; fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { has_cache=false; fun = std::forward<T>(arg); return *this; }

    size_t size() const { return (size_t) has_cache; }

    bool Has() const { return has_cache; }
    void Clear() { has_cache=false; }
    void Erase() { has_cache=false; }

    return_t operator()() {
      emp_assert(fun);
      if (has_cache == false) { cached_value = fun(); has_cache = true; }
      return cached_value;
    }

    operator bool() { return (bool) fun; }

    // A memo_function can be converted to a regular std::function for function calls.
    operator std::function<R()>() {
      return [this](){ return operator()(); };
    }
    std::function<R()> to_function() {
      return [this](){ return operator()(); };
    }
  };

}

#endif
