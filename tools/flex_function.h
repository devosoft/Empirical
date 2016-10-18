//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A flex_function works identicaly to std::function, but holds default values for
//  all parameters so any number of paramaters can be used in a call.

#ifndef EMP_FLEX_FUNCTION_H
#define EMP_FLEX_FUNCTION_H

#include <functional>
#include <tuple>

#include "assert.h"
#include "meta.h"

namespace emp {

  template <class T> class flex_function;    // Not defined.

  // Single argument functions don't need a tuple...
  template <class R, class... ARGS>
  class flex_function<R(ARGS...)> {
  public:
    using size_t = std::size_t;
    using return_t = R;
    using fun_t = std::function<R(ARGS...)>;
    using this_t = flex_function<R(ARGS...)>;

    static constexpr int num_args = sizeof...(ARGS);

  private:
    fun_t fun;
    std::tuple<ARGS...> default_args;

  public:
    template <typename T>
    flex_function(T && fun_info) : fun(std::forward<T>(fun_info)) { ; }
    flex_function(const this_t &) = default;
    flex_function(this_t &&) = default;
    flex_function() { ; }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;
    this_t & operator=(const fun_t & _f) { fun=_f; return *this; }
    this_t & operator=(fun_t && _f) { fun=std::move(_f); return *this; }
    template <typename T>
    this_t & operator=(T && arg) { fun = std::forward<T>(arg); return *this; }

  };

}

#endif
