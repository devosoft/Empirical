/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file ce_random.hpp
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *
 *  Status: DESIGN
 *
 *  Constructor:
 *    Random(int _seed=-1)
 *      _seed is the random number seed that will produce a unique pseudo-random sequence.
 *      (a value of -1 indicates that the seed should be based off of a combination of time
 *      and the memory position of the random number generator, in case multiple generators
 *      start at the same time.)
 *
 *  Other useful functions:
 *    double GetDouble()
 *    double GetDouble(double max)
 *    double GetDouble(double min, double max)
 *      Retrive a random double in the range [min, max).  By default, min=0.0 and max=1.0.
 *
 *    int GetInt(int max)
 *    int GetInt(int min, int max)
 *    uint32_t GetUInt(uint32_t max)
 *    uint32_t GetUInt(uint32_t min, uint32_t max)
 *      Retrive a random int or uint in the range [min, max).  By default, min=0.
 *
 *    bool P(double p)
 *      Tests a random value [0,1) against a given probability p, and returns true of false.
 *
 *    double GetRandNormal(const double mean, const double std)
 *    uint32_t GetRandPoisson(const double n, double p)
 *    uint32_t GetRandPoisson(const double mean)
 *    uint32_t GetRandBinomial(const double n, const double p)
 *      Draw a value from the given distributions
 */

#ifndef EMP_IN_PROGRESS_CONSTEXPR_CE_RANDOM_HPP_INCLUDE
#define EMP_IN_PROGRESS_CONSTEXPR_CE_RANDOM_HPP_INCLUDE

// #include <algorithm>
#include <climits>
#include <cmath>
#include <ctime>
#include <iterator>
#include <unistd.h>

#include "../math/math.hpp"

namespace emp {
  class Random {
  protected:
    // Internal members
    int seed;
    int original_seed;
    int inext;
    int inextp;
    int ma[56] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    // Members & functions for stat functions
    double expRV; // Exponential Random Variable for the randNormal function

    // Constants ////////////////////////////////////////////////////////////////
    // Statistical Approximation
    static const uint32_t _BINOMIAL_TO_NORMAL = 50;     // if < n*p*(1-p)
    static const uint32_t _BINOMIAL_TO_POISSON = 1000;  // if < n && !Normal approx Engine

    // Engine
    static const uint32_t _RAND_MBIG = 1000000000;
    static const uint32_t _RAND_MSEED = 161803398;

    // Internal functions

    // Setup, called on initialization and seed reset.
    constexpr void init()
    {
      // Clear variables
      for (int i = 0; i < 56; ++i) ma[i] = 0;

      int mj = (_RAND_MSEED - seed) % _RAND_MBIG;
      ma[55] = mj;
      int mk = 1;

      for (int i = 1; i < 55; ++i) {
        int ii = (21 * i) % 55;
        ma[ii] = mk;
        mk = mj - mk;
        if (mk < 0) mk += _RAND_MBIG;
        mj = ma[ii];
      }

      for (int k = 0; k < 4; ++k) {
        for (int j = 1; j < 55; ++j) {
          ma[j] -= ma[1 + (j + 30) % 55];
          if (ma[j] < 0) ma[j] += _RAND_MBIG;
        }
      }

      inext = 0;
      inextp = 31;

      // Setup variables used by Statistical Distribution functions
      expRV = -emp::Ln(Random::Get() / (double) _RAND_MBIG);
    }

    // Basic Random number
    // Returns a random number [0,_RAND_MBIG)
    constexpr uint32_t Get() {
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
     * @param _seed The seed of the random number generator.
     * A negative seed means that the random number generator gets its
     * seed from the actual system time.
     **/
    constexpr Random(const int _seed=-1) : seed(0), original_seed(0), inext(0), inextp(0), expRV(0) {
      for (int i = 0; i < 56; ++i) ma[i] = 0;
      ResetSeed(_seed);  // Calls init()
    }
    ~Random() = default;


    /**
     * @return The seed that was actually used to start the random sequence.
     **/
    constexpr int GetSeed() const { return seed; }

    /**
     * @return The seed that was originally provided by the user.
     **/
    constexpr int GetOriginalSeed() const { return original_seed; }

    /**
     * Starts a new sequence of pseudo random numbers.
     *
     * @param new_seed The seed for the new sequence.
     * A negative seed means that the random number generator gets its
     * seed from the actual system time and the process ID.
     **/
    constexpr void ResetSeed(const int new_seed) {
      original_seed = new_seed;
      seed = new_seed;
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
    constexpr double GetDouble() { return Get() / (double) _RAND_MBIG; }

    /**
     * Generate a double between 0 and a given number.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr double GetDouble(const double max) { return GetDouble() * max; }

    /**
     * Generate a double out of a given interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr double GetDouble(const double min, const double max) { return GetDouble() * (max - min) + min; }

    /**
     * Generate an uint32_t.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr uint32_t GetUInt(const uint32_t max) { return static_cast<int>(GetDouble() * static_cast<double>(max)); }

    /**
     * Generate an uint32_t out of an interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr uint32_t GetUInt(const uint32_t min, const uint32_t max) { return GetUInt(max - min) + min; }

    /**
     * Generate an int out of an interval.
     *
     * @return The pseudo random number.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr int32_t GetInt(const int max) { return static_cast<int>(GetUInt(max)); }

    /**
     * Generate an int out of an interval.
     *
     * @return The pseudo random number.
     * @param min The lower bound for the random numbers.
     * @param max The upper bound for the random numbers (will never be returned).
     **/
    constexpr int32_t GetInt(const int min, const int max) { return static_cast<int>(GetUInt(max - min)) + min; }


    // Random Event Generation //////////////////////////////////////////////////

    // P(p) => if p < [0,1) random variable
    constexpr bool P(const double _p) { return (Get() < (_p * _RAND_MBIG));}


    // Statistical functions ////////////////////////////////////////////////////

    // Distributions //

    /**
     * Generate a random variable drawn from a unit normal distribution.
     **/
    constexpr double GetRandNormal() {
      // Draw from a Unit Normal Dist
      // Using Rejection Method and saving of initial exponential random variable
      double expRV2 = -emp::Ln(GetDouble());
      while (1) {
        expRV -= (expRV2-1)*(expRV2-1)/2;
        if (expRV > 0) break;
        expRV = -emp::Ln(GetDouble());
        expRV2 = -emp::Ln(GetDouble());
      }
      if (P(.5)) return expRV2;
      return -expRV2;
    }

    /**
     * Generate a random variable drawn from a distribution with given
     * mean and standard deviation.
     **/
    constexpr double GetRandNormal(const double mean, const double std) { return mean + GetRandNormal() * std; }

    /**
     * Generate a random variable drawn from a Poisson distribution.
     *
     * @param mean The mean of the distribution.
     **/
    constexpr uint32_t GetRandPoisson(const double mean) {
      // Draw from a Poisson Dist with mean; if cannot calculate, return UINT_MAX.
      // Uses Rejection Method
      const double a = emp::Exp(-mean);
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
     * Generate a random variable drawn from a Poisson distribution.
     **/
    constexpr uint32_t GetRandPoisson(const double n, double p) {
      // Optimizes for speed and calculability using symmetry of the distribution
      if (p > .5) return (uint32_t) n - GetRandPoisson(n * (1 - p));
      else return GetRandPoisson(n * p);
    }

    /**
     * Generate a random variable drawn from a Binomial distribution.
     *
     * This function is exact, but slow.
     * @see Random::GetRandBinomial
     **/
    constexpr uint32_t GetFullRandBinomial(const double n, const double p) { // Exact
      // Actually try n Bernoulli events with probability p
      uint32_t k = 0;
      for (uint32_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
    }

    /**
     * Generate a random variable drawn from a Binomial distribution.
     *
     * This function is faster than @ref Random::GetFullRandBinomial(), but
     * uses some approximations.
     *
     * @see Random::GetFullRandBinomial
     **/
    constexpr uint32_t GetRandBinomial(const double n, const double p) { // Approx
      // Approximate Binomial if appropriate
      // if np(1-p) is large, use a Normal approx
      if (n * p * (1 - p) >= _BINOMIAL_TO_NORMAL) {
        return static_cast<uint32_t>(GetRandNormal(n * p, n * p * (1 - p)) + 0.5);
      }
      // elseif n is large, use a Poisson approx
      if (n >= _BINOMIAL_TO_POISSON) {
        uint32_t k = GetRandPoisson(n, p);
        if (k < UINT_MAX) return k; // if approx worked
      }
      // otherwise, actually generate the randBinomial
      return GetFullRandBinomial(n, p);
    }
  };


  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  /*! This is an adaptor to make Random behave like a proper STL random number
    generator.
  */
  struct RandomStdAdaptor {
    typedef int argument_type;
    typedef int result_type;

    RandomStdAdaptor(Random& rng) : _rng(rng) { }
    int operator()(int n) { return _rng.GetInt(n); }

    Random& _rng;
  };
  #endif // DOXYGEN_SHOULD_SKIP_THIS


  /*! Draw a sample (with replacement) from an input range, copying to the output range.
   */
  template <typename ForwardIterator, typename OutputIterator, typename RNG>
  void sample_with_replacement(ForwardIterator first, ForwardIterator last, OutputIterator ofirst, OutputIterator olast, RNG rng) {
    std::size_t range = std::distance(first, last);
    while(ofirst != olast) {
      *ofirst = *(first+rng(range));
      ++ofirst;
    }
  }


} // END emp namespace

#endif // #ifndef EMP_IN_PROGRESS_CONSTEXPR_CE_RANDOM_HPP_INCLUDE
