/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Binomial.h
 *  @brief A heavy-weight binomial distribution that can quickly generate random values.
 *  @note Status: ALPHA
 *
 *  @todo Consider converting this class to a more generic Distribution class
 *        (though technically it will only work with discrete distributions.)
 */

#ifndef EMP_BINOMIAL_H
#define EMP_BINOMIAL_H

#include "Random.h"
#include "UnorderedIndexMap.h"

namespace emp {

  class Binomial {
  private:
    UnorderedIndexMap weights;
  public:
    Binomial(double p, size_t N) : weights(N+1) {
      // p^k * (1-p)^(N-k) * N!/k!(N-k)!

      // Loop through all of the results and calculate their probabilities.
      for (size_t k = 0; k <= N; k++) {
        // For this k, determine its probability.
        double prob = 1.0;
        for (size_t i = 0; i < N; i++) {
          prob *= (i < k) ? p : (1.0 - p);
          prob *= (double) (N-i);
          prob /= (double) ((i < k) ? (k-i) : (N-i));
        }
        weights.Adjust(k, prob);
      }
    }

    double GetTotalProb() const { return weights.GetWeight(); }
    double operator[](size_t id) const { return weights.GetWeight(id); }

    size_t PickRandom(Random & random) const {
      return weights.Index( random.GetDouble(GetTotalProb()) );
    }
  };

}

#endif
