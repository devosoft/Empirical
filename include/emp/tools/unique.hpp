/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/unique.hpp
 * @brief Methods to generate UIDs.
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_UNIQUE_HPP_GUARD
#define INCLUDE_EMP_TOOLS_UNIQUE_HPP_GUARD

#include <functional>
#include <iostream>
#include <sstream>
#include <stddef.h>
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

  /// A function that will always return a unique string (using UniqVal) with provided
  /// prefix and name.)
  static inline std::string UniqueName(const std::string & prefix = "",
                                       const std::string & suffix = "") {
    std::stringstream ss;
    ss << prefix << UniqueVal() << suffix;
    return ss.str();
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_TOOLS_UNIQUE_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: uniq
