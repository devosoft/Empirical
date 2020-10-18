/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  flex_function.h
 *  @brief Based on std::function, but holds default parameter values for calls with fewer args.
 *  @note Status: ALPHA
 */


#ifndef EMP_FLEX_FUNCTION_H
#define EMP_FLEX_FUNCTION_H

#include <functional>
#include <tuple>

#include "../base/assert.h"
#include "../meta/meta.h"

namespace emp {

  template <class T> class flex_function;    // Not defined.

  /// A functon class that is almost identical to std::function, but is provided with default values
  /// for all parameters so that it can be called with fewer arguments, as needed.
  template <class R, class... ARGS>
  class flex_function<R(ARGS...)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using fun_t = std::function<R(ARGS...)>;
    using this_t = flex_function<R(ARGS...)>;
    using tuple_t = std::tuple<ARGS...>;

    static constexpr int num_args = sizeof...(ARGS);

  private:
    fun_t fun;               ///< Function to be called.
    tuple_t default_args;    ///< Arguments to be used if not enough are provided in call.

  public:
    template <typename T>
    flex_function(T && fun_info) : fun(std::forward<T>(fun_info)), default_args() { ; }
    flex_function(const this_t &) = default;
    flex_function(this_t &&) = default;
    flex_function() = default;

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { fun = std::forward<T>(arg); return *this; }

    /// Set the default value for a specific parameter.
    template <int ID> void SetDefault(pack_id<ID,ARGS...> & in_default) {
      std::get<ID>(default_args) = in_default;
    }

    /// Set the default values for all parameters.
    void SetDefaults(ARGS... args) {
      default_args = std::make_tuple(args...);
    }

    /// Allow the function to be called with all args.
    return_t operator()(ARGS... k) const {
      emp_assert(fun);
      return fun(k...);
    }

    /// All the function to be called with a subset of arguments (and the rest set to defaults)
    template <class... IN_ARGS>
    return_t operator()(IN_ARGS &&... k) const {
      emp_assert(fun);
      constexpr int in_args = sizeof...(IN_ARGS);
      static_assert(in_args < num_args, "This operator() should only be called if too few args provided.");
      return operator()(std::forward<IN_ARGS>(k)..., std::get<in_args>(default_args));
    }

    /// Determine whether this function has been set.
    operator bool() const { return (bool) fun; }

  };

}

#endif
