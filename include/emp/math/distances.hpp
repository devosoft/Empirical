/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2017-2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/math/distances.hpp
 * @brief Library of commonly used distance functions
 * @note Status: BETA
 */

#pragma once

#ifndef INCLUDE_EMP_MATH_DISTANCES_HPP_GUARD
#define INCLUDE_EMP_MATH_DISTANCES_HPP_GUARD

#include <stddef.h>

#include "../meta/type_traits.hpp"

#include "math.hpp"

namespace emp {

  /// Calculate Euclidean distance between two containers. If the containers contain a pointer
  /// type, the pointers will be de-referenced first.
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value, double>::type
  EuclideanDistance(C & p1, C & p2) {
    emp_assert(p1.size() == p2.size() &&
               "Cannot calculate euclidean distance between two containers of different lengths.");

    double dist = 0;
    for (size_t i = 0; i < p1.size(); ++i) { dist += pow(p1[i] - p2[i], 2); }

    return sqrt(dist);
  }

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  /// Calculate Euclidean distance between two containers of pointers (de-referencing the pointers)
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value, double>::type
  EuclideanDistance(C & p1, C & p2) {
    emp_assert(p1.size() == p2.size() &&
               "Cannot calculate euclidean distance between two containers of different lengths.");

    double dist = 0;
    for (size_t i = 0; i < p1.size(); ++i) { dist += emp::Pow(*p1[i] - *p2[i], 2); }

    return sqrt(dist);
  }

#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATH_DISTANCES_HPP_GUARD
