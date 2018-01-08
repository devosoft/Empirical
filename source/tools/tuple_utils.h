/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file tuple_utils.h
 *  @brief Functions to simplify the use of std::tuple
 *  @note Status: RELEASE
 */

#ifndef EMP_TUPLE_UTILS_H
#define EMP_TUPLE_UTILS_H

#include <functional>
#include <tuple>

#include "../meta/IntPack.h"
#include "../meta/meta.h"

namespace emp {

  /// Quick way to calculate tuple size.
  template <typename TUPLE_T>
  constexpr inline int tuple_size() { return std::tuple_size<TUPLE_T>::value; }


  /// Reorganize the entries in tuple; the provided int pack must specify the new ordering.
  template <typename... Ts, int... Ps>
  auto shuffle_tuple(const std::tuple<Ts...> & tup, IntPack<Ps...>) {
    return std::make_tuple( std::get<Ps>(tup)... );
  }


  /// Apply a tuple as arguments to a function, where all argument positions in function are
  /// specified with and IntPack
  template < typename FUN_T, typename TUPLE_T, int... N >   // Specify positions to apply...
  auto ApplyTuple(const FUN_T & fun, const TUPLE_T & tup, IntPack<N...>) {
    return fun(std::get<N>(tup)...);
  }

  /// Apply a tuple as arguments to a function, in order.
  template <typename FUN_T, typename TUPLE_T>              // Apply whole tuple
  auto ApplyTuple(const FUN_T & fun, const TUPLE_T & tup) {
    return ApplyTuple(fun, tup, IntPackRange<0,tuple_size<TUPLE_T>()>());
  }



  /// Setup tuples to be able to be used in hash tables.
  template <typename... TYPES>
  struct TupleHash {
    using tuple_t = std::tuple<TYPES...>;
    using fun_t = std::function<std::size_t(TYPES...)>;

    std::size_t operator()( const tuple_t & tup ) const {
      return ApplyTuple<fun_t, tuple_t> (emp::CombineHash<TYPES...>, tup);
    }
  };

}

#endif
