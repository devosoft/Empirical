/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file distances.h
 *  @brief Library of commonly used distance functions
 *  @note Status: BETA
 */

#ifndef EMP_DISTANCES_H
#define EMP_DISTANCES_H

#include "math.h"
#include "../meta/type_traits.h"

namespace emp {

  /// Calculate Euclidean distance between two containers
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value, double>::type
  EuclideanDistance(C & p1, C & p2) {
      emp_assert(p1.size() == p2.size() 
                && "Cannot calculate euclidean distance between two containers of different lengths.");

      double dist = 0;
      for (size_t i = 0; i < p1.size(); ++i) {
          dist += pow(p1[i] - p2[i], 2);
      }

      return sqrt(dist);
  }

  /// Calculate Euclidean distance between two containers of pointers (de-referencing the pointers)
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value, double>::type
  EuclideanDistance(C & p1, C & p2) {

      emp_assert(p1.size() == p2.size() 
                && "Cannot calculate euclidean distance between two containers of different lengths.");

      double dist = 0;
      for (size_t i = 0; i < p1.size(); ++i) {
          dist += emp::Pow(*p1[i] - *p2[i], 2);
      }

      return sqrt(dist);
  }


}

#endif