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
