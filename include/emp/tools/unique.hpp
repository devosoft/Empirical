/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  flex_function.hpp
 *  @brief A collection of broadly-useful functions (that don't fit elsewhere)
 *  @note Status: BETA (though new functions are added frequently)
 */


#ifndef EMP_UNIQUE_H
#define EMP_UNIQUE_H

#include <functional>
#include <iostream>
#include <type_traits>
#include <sstream>

#include "../base/assert.hpp"

#include "../math/constants.hpp"

namespace emp {
  /// A function that will always return a unique value (and trip an assert if it can't...)
  static size_t UniqueVal() {
    static size_t val = 0;
    emp_assert(val < MaxValue<size_t>() && "Ran out of unique values in size_t!");
    return val++;
  }

  /// A function that will always return a unique stringname (using UniqVal) with provided
  /// prefix and postfix.)
  static inline std::string UniqueName(const std::string & prefix="",
                                       const std::string & postfix="") {
    std::stringstream ss;
    ss << prefix << UniqueVal() << postfix;
    return ss.str();
  }
}



#endif
