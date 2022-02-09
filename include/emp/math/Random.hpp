/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2020.
 *
 *  @file  Random.hpp
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  @note Status: RELEASE
 */

#ifndef EMP_RANDOM_H
#define EMP_RANDOM_H

#include <algorithm>
#include <ctime>
#include <climits>
#include <cmath>
#include <cstring>
#include <iterator>

#include "../base/assert.hpp"
#include "../bits/bitset_utils.hpp"
#include "Range.hpp"

namespace emp {

namespace impl {

// implementation helper for GetRandZeroSymmetricPareto
inline double calc_p_zero_symmetric_pareto_fat_side(
  const double alpha, const double lambda, const double m, const double n
) {

  if (n == m) return 0.5;

  const double res_addend = (
    1.0 / (
      2.0
      - std::pow( lambda/(n + lambda), alpha)
      - std::pow( lambda/(m + lambda), alpha)
    )
  );

  double res_subtrahend_denom;
  res_subtrahend_denom = (
    std::pow( (m + lambda), alpha )
    * (2.0 * std::pow( lambda, -alpha) - std::pow(n + lambda, -alpha))
    - 1.0
  );
  // need a fallback way to calculate for numerical stability
  if (std::isnan(res_subtrahend_denom)) res_subtrahend_denom = (
    2.0 * std::pow( (m + lambda)/lambda, alpha )
    - std::pow( (m + lambda)/(n + lambda), alpha )
    - 1.0
  );

  const double res = res_addend - 1.0 / res_subtrahend_denom;
  emp_assert(
    std::isfinite(res)
    && std::clamp(res, 0.4999, 1.0001) == res,
    res
  );
  return res;

}

// implementation helper for GetRandZeroSymmetricPareto
inline double calc_p_zero_symmetric_pareto_skinny_side(
  const double alpha, const double lambda, const double m, const double n
) {

  if (n == m) return 0.5;

  const double res = (
      1.0
      - std::pow(lambda / (n + lambda), alpha)
    ) / (
      2.0
      - std::pow(lambda / (n + lambda), alpha)
      - std::pow(lambda / (m + lambda), alpha)
  );
  emp_assert(
    std::isfinite(res)
    && std::clamp(res, 0.0, 0.5001) == res,
    res
  );

  // assert p_fat_side and p_skinny_side are complimentary outcomes
  emp_assert( std::abs(
    1.0
    - calc_p_zero_symmetric_pareto_fat_side(alpha, lambda, m, n)
    - res
  ) < 0.001, res );

  return res;

}

} // namespace impl

  using namespace emp;

  ///  Middle Square Weyl Sequence: A versatile and non-patterned pseudo-random-number
  ///  generator.
  ///  Based on: https://en.wikipedia.org/wiki/Middle-square_method
  class Random {
  protected:

    uint64_t value = 0;                       ///< Current squaring value
    uint64_t weyl_state = 0;                  ///< Weyl sequence state
    uint64_t original_seed = 0;               ///< Seed to start sequence; initialized weyl_state

    // Members & functions for stat functions
    double expRV = 0.0;    ///< Exponential Random Variable for the randNormal function

    // Constants ////////////////////////////////////////////////////////////////
    static constexpr const uint64_t RAND_CAP = 4294967296;  // 2^32
    static constexpr const uint64_t STEP_SIZE = 0xb5ad4eceda1ce2a9;  ///< Weyl sequence step size


    /// Basic Random number
    /// Returns a random number [0, RAND_CAP)
    uint32_t Get() {
      value *= value;                       // Square the current value.
      value += (weyl_state += STEP_SIZE);   // Take a step in the Weyl sequence
      value = (value>>32) | (value<<32);    // Return the middle of the value
      return (uint32_t) value;
    }

  public:
    /// Set up the random generator object with an optional seed value.
    Random(const int seed = -1) {
      ResetSeed(seed);  // Calls init()
    }

    ~Random() { ; }

    /// Advance pseudorandom number generation engine one step.
    void StepEngine() { Get(); }

    /// @return The current seed used to initialize this pseudo-random sequence.
    inline uint64_t GetSeed() const { return original_seed; }

    /// Starts a new sequence of pseudo random numbers.  A negative seed means that the random
    /// number generator gets its seed from the current system time and the process memory.
    void ResetSeed(const int64_t seed) {
      // If the provided seed is <= 0, choose a unique seed based on time and memory location.
      if (seed <= 0) {
        uint64_t seed_time = (uint64_t) time(NULL);
        uint64_t seed_mem = (uint64_t) this;
        weyl_state = seed_time ^ seed_mem;
      }

      else weyl_state = (uint64_t) seed;

      // Save the seed that was ultimately used to start this pseudo-random sequence.
      original_seed = weyl_state;

      weyl_state *= 2;  // Make sure starting state is even.

    }


    // Random Number Generation /////////////////////////////////////////////////

    /// @return A pseudo-random double value between 0.0 and 1.0
    inline double GetDouble() { return Get() / (double) RAND_CAP; }

    /// @return A pseudo-random double value between 0.0 and max
    inline double GetDouble(const double max) { return GetDouble() * max; }

    /// @return A pseudo-random double value between min and max
    inline double GetDouble(const double min, const double max) {
      return GetDouble() * (max - min) + min;
    }

    /// @return A pseudo-random double in the provided range.
    inline double GetDouble(const Range<double> range) {
      return GetDouble(range.GetLower(), range.GetUpper());
     }


    /// @return A pseudo-random 32-bit (4 byte) unsigned int value.
    inline uint32_t GetUInt() {
      return Get();
    }

    /// @return A pseudo-random 32-bit unsigned int value between 0 and max
    template <typename T>
    inline uint32_t GetUInt(const T max) {
      return static_cast<uint32_t>(GetDouble() * static_cast<double>(max));
    }

    /// @return A pseudo-random 32-bit unsigned int value between min and max
    template <typename T1, typename T2>
    inline uint32_t GetUInt(const T1 min, const T2 max) {
      return GetUInt<uint32_t>((uint32_t) max - (uint32_t) min) + (uint32_t) min;
    }

    /// @return A pseudo-random 32-bit unsigned int value in the provided range.
    template <typename T>
    inline uint32_t GetUInt(const Range<T> range) {
      return GetUInt(range.GetLower(), range.GetUpper());
    }


    /// @return A pseudo-random 64-bit (8 byte) unsigned int value.
    inline uint64_t GetUInt64() {
      return ( static_cast<uint64_t>(GetUInt()) << 32 )
             + static_cast<uint64_t>(GetUInt());
    }

    /// @return A pseudo-random 64-bit unsigned int value between 0 and max
    inline uint64_t GetUInt64(const uint64_t max) {
      if (max <= RAND_CAP) return (uint64_t) GetUInt(max);  // Don't need extra precision.

      size_t mask = emp::MaskUsed(max);              // Create a mask for just the bits we need.
      uint64_t val = GetUInt64() & mask;             // Grab a value using just the current bits.
      while (val >= max) val = GetUInt64() & mask;   // Grab new values until we find a valid one.

      return val;
    }


    /// @return A pseudo-random 32-bit (4 byte) int value between 0 and max
    inline int32_t GetInt(const int32_t max) {
      return static_cast<int32_t>(GetUInt((uint32_t) max));
    }

    /// @return A pseudo-random 32-bit (4 byte) int value between min and max
    inline int32_t GetInt(const int min, const int max) { return GetInt(max - min) + min; }

    /// @return A pseudo-random 32-bit (4 byte) int value in range
    inline int32_t GetInt(const Range<int> range) {
      return GetInt(range.GetLower(), range.GetUpper());
    }


    /// Randomize a contiguous segment of memory.
    void RandFill(unsigned char * dest, const size_t num_bytes) {
      size_t leftover = num_bytes % 4;
      size_t limit = num_bytes - leftover;

      // Fill out random bytes in groups of four.
      for (size_t byte = 0; byte < limit; byte += 4) {
        uint32_t rnd = Get();
        std::memcpy(dest+byte, &rnd, 4);
      }

      // If we don't have a multiple of four, fill in the remaining.
      if (leftover) {
        uint32_t rnd = Get();
        std::memcpy(dest+num_bytes-leftover, &rnd, leftover);
      }
    }


    // Random Event Generation //////////////////////////////////////////////////

    /// Tests a random value [0,1) against a given probability p, and returns true of false.
    /// @param p The probability of the result being "true".
    inline bool P(const double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      return (Get() < (p * RAND_CAP));
    }


    // Statistical functions ////////////////////////////////////////////////////

    // Distributions //

    /// Generate a random variable drawn from a unit normal distribution.
    double GetRandNormal() {
      // Draw from a Unit Normal Dist
      // Using Rejection Method and saving of initial exponential random variable
      double expRV2;
      while (1) {
        expRV2 = -log(GetDouble());
        expRV -= (expRV2-1)*(expRV2-1)/2;
        if (expRV > 0) break;
        expRV = -log(GetDouble());
      }
      if (P(.5)) return expRV2;
      return -expRV2;
    }

    /// @return A random variable drawn from a normal distribution.
    /// @param mean Center of distribution.
    /// @param std Standard deviation of distribution.
    inline double GetRandNormal(const double mean, const double std) { return mean + GetRandNormal() * std; }

    /// @return A random variable drawn from a pareto distribution.
    /// @param alpha Shape parameter (default 1).
    /// @param lower_bound Lower bound (default 1).
    /// @param upper_bound Upper bound (default infinity, unbounded).
    /// See <https://en.wikipedia.org/wiki/Pareto_distribution>
    inline double GetRandPareto(
      const double alpha=1.0,
      const double lower_bound=1.0,
      const double upper_bound=std::numeric_limits<double>::infinity()
    ) {
      emp_assert( alpha > 0.0, alpha );
      emp_assert( lower_bound > 0.0, lower_bound );
      emp_assert( lower_bound <= upper_bound, lower_bound, upper_bound );
      if (lower_bound == upper_bound) return lower_bound;
      // uses inverse transform sampling method
      // see https://en.wikipedia.org/wiki/Inverse_transform_sampling
      // and https://en.wikipedia.org/wiki/Pareto_distribution

      // see https://gist.github.com/mmore500/b6de0158a8851ce6e3d65105ed35197f
      // for validation and testing
      // url has also been archived on the archive.org wayback machine

      const double unif_lb = std::pow(lower_bound / upper_bound, alpha);
      constexpr double unif_ub = 1.0;
      const double unif_sample = GetDouble(unif_lb, unif_ub);

      return lower_bound / std::pow(unif_sample, 1.0/alpha);
    }

    /// @return A random variable drawn from a pareto distribution.
    /// @param alpha Shape parameter (default 1).
    /// @param lambda Scale parameter (default 1).
    /// @param upper_bound Upper bound (default infinity, unbounded).
    /// See <https://en.wikipedia.org/wiki/Lomax_distribution>
    inline double GetRandLomax(
      const double alpha=1.0,
      const double lambda=1.0,
      const double upper_bound=std::numeric_limits<double>::infinity()
    ) {
      emp_assert( alpha > 0.0, alpha );
      emp_assert( lambda > 0.0, lambda );
      emp_assert( upper_bound >= 0.0, upper_bound );
      return GetRandPareto(alpha, lambda, upper_bound + lambda) - lambda;
    }

    /// @return A random variable drawn from a distribution with symmetric
    /// pareto tails extending both positively and negatively from zero.
    /// @param alpha Shape parameter (default 1).
    /// @param lambda Scale parameter (default 1).
    /// @param lower_bound Lower bound (default -infinity, unbounded).
    /// @param upper_bound Upper bound (default infinity, unbounded).
    /// Inspired by <https://doi.org/10.1109/TBC.2004.834013>
    inline double GetRandZeroSymmetricPareto(
      const double alpha=1.0,
      const double lambda=1.0,
      const double lower_bound=-std::numeric_limits<double>::infinity(),
      const double upper_bound=std::numeric_limits<double>::infinity()
    ) {
      emp_assert( alpha > 0.0, alpha );
      emp_assert( lower_bound <= 0, lower_bound );
      emp_assert( upper_bound >= 0, upper_bound );

      // see https://gist.github.com/mmore500/2b95ef3279e49fc1780b3b319c35083a
      // for derivations and validation
      // url has also been archived on the archive.org wayback machine

      const double n = std::min(std::abs(lower_bound), upper_bound);
      const double m = std::max(std::abs(lower_bound), upper_bound);

      // probability of picking on the skinny side of zero
      const double p_skinny_side
        = impl::calc_p_zero_symmetric_pareto_skinny_side(
          alpha, lambda, m, n
        );

      // is the fat side of the distribution negative?
      const bool skinny_side_is_positive = std::abs(lower_bound) > upper_bound;

      // pick which side of zero to draw from then do draw with a lomax dist
      if (P(p_skinny_side) == skinny_side_is_positive) {
        // draw from positive side
        return GetRandLomax(alpha, lambda, upper_bound);
      } else {
        // draw from negative side
        return -GetRandLomax(alpha, lambda, std::abs(lower_bound));
      }

    }

    /// Generate a random variable drawn from a Poisson distribution.
    inline uint32_t GetRandPoisson(const double n, const double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      // Optimizes for speed and calculability using symetry of the distribution
      if (p > .5) return (uint32_t) n - GetRandPoisson(n * (1 - p));
      else return GetRandPoisson(n * p);
    }

    /// Generate a random variable drawn from a Poisson distribution.
    inline uint32_t GetRandPoisson(const double mean) {
      // Draw from a Poisson Dist with mean; if cannot calculate, return UINT_MAX.
      // Uses Rejection Method
      const double a = exp(-mean);
      if (a <= 0) return UINT_MAX; // cannot calculate, so return UINT_MAX
      uint32_t k = 0;
      double u = GetDouble();
      while (u >= a) {
        u *= GetDouble();
        ++k;
      }
      return k;
    }

    /// Generate a random variable drawn from a Binomial distribution.
    ///
    /// This function is exact, but slow.
    /// @see Random::GetApproxRandBinomial
    /// @see emp::Binomial in source/tools/Distribution.h
    inline uint32_t GetRandBinomial(const double n, const double p) { // Exact
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Actually try n Bernoulli events, each with probability p
      uint32_t k = 0;
      for (uint32_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
    }

    inline uint32_t GetRandGeometric(double p){
      emp_assert(p >= 0 && p <= 1, "Pobabilities must be between 0 and 1");
      // TODO: When we have warnings, add one for passing a really small number to
      // this function. Alternatively, make this function not ludicrously slow with small numbers.
      // Looks like return floor(ln(GetDouble())/ln(1-p)) might be sufficient?
      if (p == 0) {
        return std::numeric_limits<uint32_t>::infinity();
      }
      uint32_t result = 1;
      while (!P(p)) { result++;}
      return result;
    }

  };


  /// This is an adaptor to make Random behave like a proper STL random number generator.
  struct RandomStdAdaptor {
    typedef int argument_type;
    typedef int result_type;

    RandomStdAdaptor(Random& rng) : _rng(rng) { }
    int operator()(int n) { return _rng.GetInt(n); }

    Random& _rng;
  };


  /// Draw a sample (with replacement) from an input range, copying to the output range.
  template <typename ForwardIterator, typename OutputIterator, typename RNG>
  void sample_with_replacement(ForwardIterator first, ForwardIterator last, OutputIterator ofirst, OutputIterator olast, RNG rng) {
    std::size_t range = std::distance(first, last);
    while(ofirst != olast) {
      *ofirst = *(first+rng(range));
      ++ofirst;
    }
  }


} // END emp namespace

#endif
