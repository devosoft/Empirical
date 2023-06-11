/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file unique.hpp
 *  @brief Methods to generate UIDs.
 */

#ifndef EMP_TOOLS_UNIQUE_HPP_INCLUDE
#define EMP_TOOLS_UNIQUE_HPP_INCLUDE

#include <functional>
#include <iostream>
#include <sstream>
#include <type_traits>

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

} // namespace emp

#endif // #ifndef EMP_TOOLS_UNIQUE_HPP_INCLUDE
