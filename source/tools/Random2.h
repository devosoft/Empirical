/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2020.
 *
 *  @file  Random.h
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  @note Status: RELEASE
 */

#ifndef EMP_RANDOM2_H
#define EMP_RANDOM2_H

#include <ctime>
#include <climits>
#include <cmath>
#include <cstring>
#include <iterator>

#include "../base/assert.h"
#include "bitset_utils.h"
#include "Range.h"

namespace emp2 {
  using namespace emp;

  ///  A versatile and non-patterned pseudo-random-number generator (Mersenne Twister).
  class Random {
  protected:

    uint64_t value = 0;                       ///< Current squaring value
    uint64_t weyl_state = 0;                  ///< Weyl sequence state
    uint64_t step_size = 0xb5ad4eceda1ce2a9;  ///< Weyl sequence step size
    uint64_t original_seed = 0;               ///< Seed to start sequence; initialized weyl_state

    // Members & functions for stat functions
    double expRV = 0.0;    ///< Exponential Random Variable for the randNormal function

    // Constants ////////////////////////////////////////////////////////////////
    static constexpr const uint64_t _RAND_MAX = 4294967296;


    /// Basic Random number
    /// Returns a random number [0,_RAND_MAX)
    uint32_t Get() {
      value *= value;                       // Square the current value.
      value += (weyl_state += step_size);   // Take a step in the Weyl sequence
      value = (value>>32) | (value<<32);    // Return the middle of the value
      return (uint32_t) value;
    }

  public:
    /**
     * Set up the random generator object.
     * @param seed The seed of the random number generator.  A negative seed means that the
     * random number generator gets its seed from a combination of the actual system time and
     * the memory position of the random number generator.
     **/
    Random(const int seed = -1) {
      ResetSeed(seed);  // Calls init()
    }

    ~Random() { ; }


    /**
     * @return The current state of the seed in the random sequence.
     **/
    inline uint64_t GetSeed() const { return weyl_state; }

    /**
     * @return The seed that was originally provided by the user.
     **/
    inline uint64_t GetOriginalSeed() const { return original_seed; }

    /**
     * Starts a new sequence of pseudo random numbers.
     *
     * @param new_seed The seed for the new sequence.
     * A negative seed means that the random number generator gets its
     * seed from the actual system time and the process ID.
     **/
    inline void ResetSeed(const int seed) {
      // If the provided seed is <= 0, choose a unique seed based on time and memory location.
      if (seed <= 0) {
        uint64_t seed_time = (uint64_t) time(NULL);
        uint64_t seed_mem = (uint64_t) this;
        weyl_state = seed_time ^ seed_mem;
      }

      else weyl_state = (uint64_t) seed;

      weyl_state *= 2;  // Make sure starting state is even.

      original_seed = weyl_state;
    }


    // Random Number Generation /////////////////////////////////////////////////

    /**
     * Generate a double between 0.0 and 1.0
     *
     * @return The pseudo random number.
     **/
    inline double GetDouble() { return Get() / (double) _RAND_MAX; }

    /**
     * Generate a double between 0 and a given number.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    inline double GetDouble(const double max) {
      return GetDouble() * max;
    }

    /**
     * Generate a double out of a given interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    inline double GetDouble(const double min, const double max) {
      return GetDouble() * (max - min) + min;
    }

    /**
     * Generate a double out of a given interval.
     *
     * @return The pseudo random number.
     * @param range The upper and lower bounds for the random numbers [lower, upper)
     **/
    inline double GetDouble(const Range<double> range) {
      return GetDouble(range.GetLower(), range.GetUpper());
     }

    /**
     * Generate an uint32_t.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    template <typename T>
    inline uint32_t GetUInt(const T max) {
      return static_cast<uint32_t>(GetDouble() * static_cast<double>(max));
    }

    /**
     * Generate a random 32-bit block of bits.
     *
     * @return The pseudo random number.
     **/
    inline uint32_t GetUInt() {
      return Get();
    }

    /**
     * Generate a random 64-bit block of bits.
     *
     * @return The pseudo random number.
     **/
    inline uint64_t GetUInt64() {
      // @MAM profiled,
      // this is faster than using RandFill
      // https://gist.github.com/mmore500/8747e456b949b5b18b3ee85dd9b4444d
      return ( static_cast<uint64_t>(GetUInt()) << 32 )
             + static_cast<uint64_t>(GetUInt());
    }


    /// Randomize a contiguous segment of memory.

    inline void RandFill(unsigned char * dest, const size_t num_bytes) {
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

    /**
     * Generate an uint64_t.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     * @todo this function needs to be tested and refined.
     **/
    inline uint64_t GetUInt64(const uint64_t max) {
      if (max <= _RAND_MAX) return (uint64_t) GetUInt(max);  // Don't need extra precision.

      size_t mask = emp::MaskUsed(max);              // Create a mask for just the bits we need.
      uint64_t val = GetUInt64() & mask;             // Grab a value using just the current bits.
      while (val >= max) val = GetUInt64() & mask;   // Grab new values until we find a valid one.

      return val;
    }


    /**
     * Generate an uint32_t out of an interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    template <typename T1, typename T2>
    inline uint32_t GetUInt(const T1 min, const T2 max) {
      return GetUInt<uint32_t>((uint32_t) max - (uint32_t) min) + (uint32_t) min;
    }

    /**
     * Generate a uint32_t out of a given interval.
     *
     * @return The pseudo random number.
     * @param range The upper and lower bounds for the random numbers [lower, upper)
     **/
    template <typename T>
    inline uint32_t GetUInt(const Range<T> range) {
      return GetUInt(range.GetLower(), range.GetUpper());
    }

    /**
     * Generate an int out of an interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    inline int GetInt(const int max) { return static_cast<int>(GetUInt((uint32_t) max)); }
    inline int GetInt(const int min, const int max) { return GetInt(max - min) + min; }
    inline int GetInt(const Range<int> range) { return GetInt(range.GetLower(), range.GetUpper()); }


    // Random Event Generation //////////////////////////////////////////////////

    /// Tests a random value [0,1) against a given probability p, and returns true of false.
    /// @param p The probability of the result being "true".
    inline bool P(const double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      return (Get() < (p * _RAND_MAX));
    }


    // Statistical functions ////////////////////////////////////////////////////

    // Distributions //

    /**
     * Generate a random variable drawn from a unit normal distribution.
     **/
    inline double GetRandNormal() {
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

    /**
     * Generate a random variable drawn from a distribution with given
     * mean and standard deviation.
     **/
    inline double GetRandNormal(const double mean, const double std) { return mean + GetRandNormal() * std; }

    /**
     * Generate a random variable drawn from a Poisson distribution.
     **/
    inline uint32_t GetRandPoisson(const double n, double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      // Optimizes for speed and calculability using symetry of the distribution
      if (p > .5) return (uint32_t) n - GetRandPoisson(n * (1 - p));
      else return GetRandPoisson(n * p);
    }

    /**
     * Generate a random variable drawn from a Poisson distribution.
     *
     * @param mean The mean of the distribution.
     **/
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

    /**
     * Generate a random variable drawn from a Binomial distribution.
     *
     * This function is exact, but slow.
     * @see Random::GetApproxRandBinomial
     * @see emp::Binomial in source/tools/Distribution.h
     **/
    inline uint32_t GetRandBinomial(const double n, const double p) { // Exact
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Actually try n Bernoulli events, each with probability p
      uint32_t k = 0;
      for (uint32_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
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
