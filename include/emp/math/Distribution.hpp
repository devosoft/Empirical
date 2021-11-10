/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2020.
 *
 *  @file Distribution.hpp
 *  @brief A set of pre-calculated discrete distributions that can quickly generate random values.
 *  @note Status: ALPHA
 *
 *  A Distribution is a pre-calculated set of probabilities to quickly pick a whole-number result.
 *  These should be used when either we need to draw from the same distribution many time (and hence
 *  the extra time to pre-calculate it is amortized away) -or- in functions that we want to call with
 *  a range of distributions that we may not know ahead of time.
 *
 *  Currently, we have:
 *
 *    Uniform - All values in a range are equally likelty to be picked.
 *    Binomial - How many successes with p probability will occur in N attempts?
 *    NegativeBinomial - How many attempts to reach N successes, with p probability per attempt?
 *
 *
 *  Developor Notes:
 *  - We should setup an offset in the base Distribution class to ignore "impossible" low values.
 *
 */

#ifndef EMP_MATH_DISTRIBUTION_HPP_INCLUDE
#define EMP_MATH_DISTRIBUTION_HPP_INCLUDE

#include "../datastructs/UnorderedIndexMap.hpp"

#include "Random.hpp"

namespace emp {

  class Distribution {
  protected:
    UnorderedIndexMap weights;

  public:
    size_t GetSize() const { return weights.GetSize(); }
    double GetTotalProb() const { return weights.GetWeight(); }
    double operator[](size_t id) const { return weights.GetWeight(id); }

    /// Pick an item from a distribution using a value between 0.0 and 1.0.
    size_t PickPosition(double in_value) {
      emp_assert(in_value >= 0.0 && in_value <= 1.0, in_value);
      return weights.Index( in_value * GetTotalProb() );
    }

    size_t PickRandom(Random & random) const {
      emp_assert(weights.GetSize() > 0, "Distribution can only pick a random entry if it has at least one entry!");
      return weights.Index( random.GetDouble(GetTotalProb()) );
    }
  };

  class Uniform : public Distribution {
  private:
    size_t min_val = 0;  // Inclusive
    size_t max_val = 0;  // Exclusive

  public:
    Uniform(size_t _min, size_t _max) { Setup(_min, _max); }

    size_t GetMin() const { return min_val; }
    size_t GetMax() const { return max_val; }

    void Setup(size_t _min, size_t _max) {
      emp_assert(_min < _max);

      // If we're not changing these values, it's already setup!
      if (min_val == _min && max_val == _max) return;

      min_val = _min;
      max_val = _max;

      size_t num_vals = max_val - min_val;
      double val_prob = 1.0 / (double) num_vals;

      weights.Resize(max_val);
      for (size_t k = min_val; k < max_val; k++) {
        weights.Adjust(k, val_prob);
      }
    }
  };

  /// How many successes with p probability and N attempts?
  class Binomial : public Distribution {
  private:
    double p = 0.0;
    size_t N = 0;

  public:
    Binomial() { }
    Binomial(double _p, size_t _N) { Setup(_p, _N); }

    double GetP() const { return p; }
    double GetN() const { return N; }

    void Setup(double _p, size_t _N) {
      emp_assert(_p >= 0.0 && _p <= 1.0);

      // If we're not changing these values, it's already setup!
      if (p == _p && N == _N) return;

      p = _p;
      N = _N;
      weights.Resize(N+1);
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

  };

  /// How many attempts to reach N successes, assumming p probability per attempt?
  class NegativeBinomial : public Distribution {
  private:
    double p = 0.0;
    size_t N = 0;

  public:
    NegativeBinomial(double _p, size_t _N) { Setup(_p, _N); }

    double GetP() const { return p; }
    double GetN() const { return N; }

    void Setup(double _p, size_t _N) {
      // If we're not changing these values, it's already setup!
      if (p == _p && N == _N) return;

      p = _p;
      N = _N;
      emp_assert(p > 0.0 && p <= 1.0, p);
      emp_assert(N > 0, N);

      // Track the probability of each number of successes at each point in time.
      emp::vector<double> cur_probs(N, 0.0);
      cur_probs[0] = 1.0;        // Initially we start with zero successes.
      double found_probs = 0.0;  // Tally the total probability found so far.
      double q = 1.0 - p;        // Probability of failure.

      emp::vector<double> outcome_probs(1, 0.0);

      while (found_probs < 0.999999 || cur_probs[N-1] > 0.0000000001) {
        double next_prob = cur_probs[N-1] * p;  // Probability of being one short + new success!
        outcome_probs.push_back(next_prob);
        found_probs += next_prob;

        // Update all of the other probabilities.
        for (size_t i = N-1; i > 0; i--) {
          cur_probs[i] = cur_probs[i] * q + cur_probs[i-1] * p;
        }
        cur_probs[0] = cur_probs[0] * q;
      }

      weights.Adjust(outcome_probs);
    }

  };

}

#endif // #ifndef EMP_MATH_DISTRIBUTION_HPP_INCLUDE
