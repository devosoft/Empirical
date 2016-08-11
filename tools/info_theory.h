//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Tools to calculate Information Theory metrics.

#ifndef EMP_INFO_THEORY_H
#define EMP_INFO_THEORY_H

#include <cmath>

#include "vector.h"

namespace emp {

  // Entropy assumes that you are given a vector of weights, calculates the probability of
  // each, and returns the entropy.
  template<typename CONTAINER>
  double Entropy(const CONTAINER & weights) {
    double total = 0.0;
    double entropy = 0.0;
    for (auto w : weights) total += w;
    for (auto w : weights) {
      double p = ((double) w) / total;
      entropy -= p * log2(p);
    }
    return entropy;
  }

  // Allow for entropy of arbitrary objects with a converter.
  template<typename CONTAINER, typename FUNCTION>
  double Entropy(const CONTAINER & objs, FUNCTION fun) {
    double total = 0.0;
    double entropy = 0.0;
    for (auto & o : objs) total += fun(o);
    for (auto & o : objs) {
      double p = ((double) fun(o)) / total;
      entropy -= p * log2(p);
    }
    return entropy;
  }
}

#endif
