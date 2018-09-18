/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Random.h
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  @note Status: RELEASE
 */

#ifndef EMP_RANDOM_H
#define EMP_RANDOM_H

#include <ctime>
#include <climits>
#include <cmath>
#include <iterator>

#include "../base/assert.h"
#include "Range.h"

namespace emp {

  ///  A versatile and non-patterned pseudo-random-number generator (Mersenne Twister).
  class Random {
  protected:
    int seed;           ///< Current random number seed.
    int original_seed;  ///< Orignal random number seed when object was first created.
    int inext;          ///< First position in use in internal state.
    int inextp;         ///< Second position in use in internal state.
    int ma[56];         ///< Internal state of RNG

    // Members & functions for stat functions
    double expRV; // Exponential Random Variable for the randNormal function

    // Constants ////////////////////////////////////////////////////////////////
    // Statistical Approximation
    static const int32_t _BINOMIAL_TO_NORMAL = 50;     // if < n*p*(1-p)
    static const int32_t _BINOMIAL_TO_POISSON = 1000;  // if < n && !Normal approx Engine

    // Engine
    static const int32_t _RAND_MBIG = 1000000000;
    static const int32_t _RAND_MSEED = 161803398;

    // Internal functions

    // Setup, called on initialization and seed reset.
    void init()
    {
      // Clear variables
      for (int i = 0; i < 56; ++i) ma[i] = 0;

      int32_t mj = (_RAND_MSEED - seed) % _RAND_MBIG;
      ma[55] = mj;
      int32_t mk = 1;

      for (int32_t i = 1; i < 55; ++i) {
        int32_t ii = (21 * i) % 55;
        ma[ii] = mk;
        mk = mj - mk;
        if (mk < 0) mk += _RAND_MBIG;
        mj = ma[ii];
      }

      for (int32_t k = 0; k < 4; ++k) {
        for (int32_t j = 1; j < 55; ++j) {
          ma[j] -= ma[1 + (j + 30) % 55];
          if (ma[j] < 0) ma[j] += _RAND_MBIG;
        }
      }

      inext = 0;
      inextp = 31;

      // Setup variables used by Statistical Distribution functions
      expRV = -log(Random::Get() / (double) _RAND_MBIG);
    }

    // Basic Random number
    // Returns a random number [0,_RAND_MBIG)
    int32_t Get() {
      if (++inext == 56) inext = 0;
      if (++inextp == 56) inextp = 0;
      int mj = ma[inext] - ma[inextp];
      if (mj < 0) mj += _RAND_MBIG;
      ma[inext] = mj;

      return mj;
    }

  public:
    /**
     * Set up the random generator object.
     * @param _seed The seed of the random number generator.  A negative seed means that the
     * random number generator gets its seed from a combination of the actual system time and
     * the memory position of the random number generator.
     **/
    Random(const int _seed = -1) : seed(0), original_seed(0), inext(0), inextp(0), expRV(0) {
      for (int i = 0; i < 56; ++i) ma[i] = 0;
      ResetSeed(_seed);  // Calls init()
    }

    ~Random() { ; }


    /**
     * @return The seed that was actually used to start the random sequence.
     **/
    inline int GetSeed() const { return seed; }

    /**
     * @return The seed that was originally provided by the user.
     **/
    inline int GetOriginalSeed() const { return original_seed; }

    /**
     * Starts a new sequence of pseudo random numbers.
     *
     * @param new_seed The seed for the new sequence.
     * A negative seed means that the random number generator gets its
     * seed from the actual system time and the process ID.
     **/
    inline void ResetSeed(const int _seed) {
      original_seed = _seed;

      if (_seed <= 0) {
        int seed_time = (int) time(NULL);
        int seed_mem = (int) ((uint64_t) this);
        seed = seed_time ^ seed_mem;
      } else {
        seed = _seed;
      }

      if (seed < 0) seed *= -1;
      seed %= _RAND_MSEED;

      init();
    }


    // Random Number Generation /////////////////////////////////////////////////

    /**
     * Generate a double between 0.0 and 1.0
     *
     * @return The pseudo random number.
     **/
    inline double GetDouble() { return Get() / (double) _RAND_MBIG; }

    /**
     * Generate a double between 0 and a given number.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    inline double GetDouble(const double max) {
      // emp_assert(max <= (double) _RAND_MBIG, max, (double) _RAND_MBIG);  // Precision will be too low past this point...
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
      emp_assert((max-min) <= (double) _RAND_MBIG, min, max);  // Precision will be too low past this point...
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
      emp_assert(max <= (T) _RAND_MBIG, max);  // Precision will be too low past this point...
      return static_cast<uint32_t>(GetDouble() * static_cast<double>(max));
    }

    /**
     * Generate a random 32-bit block of bits.
     *
     * @return The pseudo random number.
     **/
    inline uint32_t GetUInt() {
      return ( static_cast<uint32_t>(GetDouble() * 65536.0) << 16 )
             + static_cast<uint32_t>(GetDouble() * 65536.0);
    }

    /**
     * Generate an uint64_t.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     * @todo this function needs to be tested and refined.
     **/
    template <typename T>
    inline uint64_t GetUInt64(const T max) {
      if (max <= (T) _RAND_MBIG) return (uint64_t) GetUInt(max);  // Don't need extra precision.
      const double max2 = ((double) max) / (double) _RAND_MBIG;
      emp_assert(max2 <= (T) _RAND_MBIG, max);  // Precision will be too low past this point...

      return static_cast<uint64_t>(GetDouble() * static_cast<double>(max))
           + static_cast<uint64_t>(GetDouble() * static_cast<double>(max2) * _RAND_MBIG);
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
      return (Get() < (p * _RAND_MBIG));
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
      if (p > .5) return (uint32_t)n - GetRandPoisson(n * (1 - p));
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
     * @see emp::Binomial in source/tools/Binomial.h
     **/
    inline uint32_t GetFullRandBinomial(const double n, const double p) { // Exact
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Actually try n Bernoulli events, each with probability p
      uint32_t k = 0;
      for (uint32_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
    }


    /**
     * Generate a random variable drawn from a Binomial distribution.
     *
     * This function is faster than @ref Random::GetFullRandBinomial(), but
     * uses some approximations.  Note that for repeated calculations with
     * the same n and p, the Binomial class provides a much faster and more
     * exact interface.
     *
     * @see Random::GetFullRandBinomial
     * @see emp::Binomial in source/tools/Binomial.h
     **/
    inline uint32_t GetApproxRandBinomial(const double n, const double p) { // Approx
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Approximate Binomial if appropriate

      // if np(1-p) is large, we might be tempted to use a Normal approx, but it is giving poor results.
      // if (n * p * (1 - p) >= _BINOMIAL_TO_NORMAL) {
      //   return static_cast<uint32_t>(GetRandNormal(n * p, n * p * (1 - p)) + 0.5);
      // }

      // If n is large, use a Poisson approx
      if (n >= _BINOMIAL_TO_POISSON) {
        uint32_t k = GetRandPoisson(n, p);
        if (k < UINT_MAX) return k; // if approx worked
      }

      // otherwise, actually generate the randBinomial
      return GetFullRandBinomial(n, p);
    }

    /**
     * By default GetRandBinomial calls the full (non-approximation) version.
     * 
     * Note that if approximations are okay, they can create a big speedup
     * for n > 1000.
     * 
     * @see Random::GetFullRandBinomial
     * @see Random::GetApproxRandBinomial
     * @see emp::Binomial in source/tools/Binomial.h
     **/

    inline uint32_t GetRandBinomial(const double n, const double p) {
      return GetFullRandBinomial(n,p);
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
