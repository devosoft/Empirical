/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019
 *
 *  @file  memo_function.h
 *  @brief A function that memorizes previous results to speed up any repeated calls.
 *  @note Status: BETA
 */

#ifndef EMP_MEMO_FUNCTIONS_H
#define EMP_MEMO_FUNCTIONS_H

#include <unordered_map>

#include "../base/assert.h"
#include "../meta/meta.h"

#include "tuple_utils.h"

namespace emp {

  /// Identical to std::function, but memorizes prior results (caches them) so that the function
  /// doesn't have to process those results again.  (note: Genetic version is undefinined; must have
  /// a function signature.)
  template <class T> class memo_function;    // Not defined.

  /// The simplest form of a memoized function with a single argument that is used as the cache
  /// key (no need for more complex caching).
  template <class R, class ARG>
  class memo_function<R(ARG)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using index_t = std::decay_t<ARG>;
    using fun_t = std::function<R(ARG)>;
    using this_t = memo_function<R(ARG)>;

  private:
    mutable std::unordered_map<index_t, return_t> cache_map; ///< Cached results.
    fun_t fun;                                               ///< Function to call.

  public:
    template <typename T>
    memo_function(T && fun_info) : cache_map(), fun(std::forward<T>(fun_info)) { ; }
    memo_function(const this_t &) = default;
    memo_function(this_t &&) = default;
    memo_function() : cache_map(), fun() { ; }

    /// Copy another memo_function of the same type.
    this_t & operator=(const this_t &) = default;

    /// Move to here another memo function of the same type.
    this_t & operator=(this_t &&) = default;

    /// Set a new std::function of the appropriate type.
    this_t & operator=(const fun_t & _f) { cache_map.clear(); fun=_f; return *this; }

    /// Move to here an std::function of the appropriate type.
    this_t & operator=(fun_t && _f) { cache_map.clear(); fun=std::move(_f); return *this; }
    template <typename T>

    /// A universal copy/move for other combinations that work with std::function.
    this_t & operator=(T && arg) { cache_map.clear(); fun = std::forward<T>(arg); return *this; }

    /// How many values have been cached?
    size_t size() const { return cache_map.size(); }

    /// Test if a certain input has been cached.
    bool Has(const ARG & k) const { return cache_map.find(k) != cache_map.end(); }

    /// Clear out the cache.
    void Clear() { cache_map.clear(); }

    /// Erase a specific entry from the cache.
    void Erase(const ARG & k) { cache_map.erase(k); }

    /// Call the memo_function.
    template <class KEY>
    return_t operator()(KEY && k) const {
      emp_assert(fun);
      auto cache_it = cache_map.find(k);
      if (cache_it != cache_map.end()) return cache_it->second;
      const return_t result = fun(k);
      return cache_map.emplace(std::forward<KEY>(k), result).first->second;
    }

    /// Identify if the memo_function has been set.
    operator bool() const { return (bool) fun; }

    /// Convert a memo_function to a regular std::function for function calls.
    operator std::function<R(ARG)>() {
      return [this](const ARG & arg){ return operator()(arg); };
    }

    /// More explicit conversion of a memo_function to a regular std::function for function calls.
    std::function<R(ARG)> to_function() {
      return [this](const ARG & arg){ return operator()(arg); };
    }
  };

  /// Memoize functions for when we have more than one argument...  we need to convert inputs
  /// to a tuple to make this work.
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
    memo_function(Ts &&... args) : cache_map(), fun(std::forward<Ts>(args)...) { ; }
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

  /// Zero argument functions are trivial (since they need to cache only a single value),
  /// but should still work.
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
    memo_function(T && fun_info) : cached_value(), has_cache(false), fun(std::forward<T>(fun_info)) { ; }
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
