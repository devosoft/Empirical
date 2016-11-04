//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains a set of simple functions to work with std::tuple

#ifndef EMP_TUPLE_UTILS_H
#define EMP_TUPLE_UTILS_H

#include <functional>
#include <tuple>

#include "../meta/meta.h"

namespace emp {

  // Apply a tuple as arguments to a function!
  // Unroll all IDs for the tuple, then get all of them at once, calling function.
  // Based on Kerrek SB in http://stackoverflow.com/questions/10766112/c11-i-can-go-from-multiple-args-to-tuple-but-can-i-go-from-tuple-to-multiple

  namespace {
    template <typename FUN_T, typename TUPLE_T, bool is_done, int TOTAL, int... N>
    struct apply_impl {
      static auto apply(FUN_T & fun, const TUPLE_T & tup) {
        constexpr auto num_ids = sizeof...(N);
        constexpr bool done = (TOTAL==1+num_ids);
        return apply_impl<FUN_T, TUPLE_T, done, TOTAL, N..., num_ids>::apply(fun, tup);
      }
    };

    template <typename FUN_T, typename TUPLE_T, int TOTAL, int... N>
    struct apply_impl<FUN_T, TUPLE_T, true, TOTAL, N...> {
      static auto apply(FUN_T & fun, const TUPLE_T & tup) {
        return fun(std::get<N>(tup)...);
      }
    };
  }

  // User invokes ApplyTuple
  template <typename FUN_T, typename TUPLE_T>
  auto ApplyTuple(FUN_T fun, const TUPLE_T & tup) {
    using tuple_decay_t = std::decay_t<TUPLE_T>;
    constexpr auto tup_size = std::tuple_size<tuple_decay_t>::value;
    return apply_impl<FUN_T, TUPLE_T, tup_size==0, tup_size>::apply(fun, tup);
  }

  template <typename... TYPES>
  struct TupleHash {
    using tuple_t = std::tuple<TYPES...>;
    using fun_t = std::function<std::size_t(TYPES...)>;

    std::size_t operator()( const tuple_t & tup ) const {
      return ApplyTuple<fun_t, tuple_t> (emp::CombineHash<TYPES...>, tup);
    }
  };

};

#endif
