//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Tools to calculate Information Theory metrics.
//  Status: ALPHA
//
//
//  Info-theory formulas:
//    H(X)   = -SUM(X: p[x] log2 p[x])
//    H(X|Y) = H(XY) - H(Y)
//    I(X:Y) = H(X) - H(X|Y)
//    H2(p)  = -p log2(p) - (1-p)log2(1-p)  = H({p, 1-p})
//
//  Developer notes:
//  * Input may come as WEIGHTS or as ELEMENTS (or both!).
//    ELEMENTS need to be converted to WEIGHTS for calculations.
//  * Want basic info theory functions, as well as tools (for channels, error-correction,
//    compression, etc.)

#ifndef EMP_INFO_THEORY_H
#define EMP_INFO_THEORY_H

#include "../base/vector.h"
#include "math.h"

namespace emp {

  /// Convert a vector of weights to probabilities and return the entropy of the system.
  template<typename CONTAINER>
  double Entropy(const CONTAINER & weights) {
    double total = 0.0;
    double entropy = 0.0;
    for (auto w : weights) total += w;
    for (auto w : weights) {
      double p = ((double) w) / total;
      entropy -= p * Log2(p);
    }
    return entropy;
  }

  /// Calculate the entropy in a container of arbitrary objects.
  /// Args are a container, a function to extract the weight of each member, and an (optional) total weight.
  template<typename CONTAINER, typename WEIGHT_FUN>
  double Entropy(const CONTAINER & objs, WEIGHT_FUN fun, double total=0.0) {
    // If we don't know the total, calculate it.
    if (total == 0.0) for (auto & o : objs) total += (double) fun(o);
    if (total == 0) {
      return 0;
    }

    double entropy = 0.0;
    for (auto & o : objs) {
      double p = ((double) fun(o)) / total;
      entropy -= p * Log2(p);
    }
    return entropy;
  }

  /// Calculate the entropy when their are two possibile states based on one state's probability.
  constexpr double Entropy2(const double p) {
    return -(p * Log2(p) + (1.0-p)*Log2(1.0-p));
  }

  /// Conitional Entropy: H(X|Y)
  /// Allow for entropy of arbitrary objects with a converter.
  template<typename CONTAINER, typename CAT_FUN_X, typename CAT_FUN_Y, typename WEIGHT_FUN>
  double Entropy(const CONTAINER & objs, CAT_FUN_X funX, CAT_FUN_Y funY, WEIGHT_FUN funW) {
    // @CAO Categorize all XY and all Y (maybe with helper function?) and count each.
    // @CAO Run each through Entropy function.

    double total = 0.0;
    double entropy = 0.0;
    for (auto & o : objs) total += fun(o);
    for (auto & o : objs) {
      double p = ((double) fun(o)) / total;
      entropy -= p * Log2(p);
    }
    return entropy;
  }
}

#endif
