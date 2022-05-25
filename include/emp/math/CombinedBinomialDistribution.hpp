/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2022.
 *
 *  @file CombinedBinomialDistribution.hpp
 *  @brief A means of quickly generating binomial random variables while only storing a small number of distributions.
 *  @note Status: ALPHA
 *
 *  Quick check for theory: https://math.stackexchange.com/questions/1176385/sum-of-two-independent-binomial-variables
 *
 *  If we want to generate binomial random variables of various trial counts (n's) using the
 *  Distribution class, we'd have to create a new Distribution for each unique trial count.
 *
 *  This class leverages the fact that B(n, p) + B(m, p) = B(n + m, p) to calculate binomial
 *  draws with arbitrary trail counts without storing N distributions.
 *  By storing distributions for powers of 2, we only store log_2(N) distributions.
 *
 *  Developor Notes:
 *    - We should come up with a more informative name for the file/class
 */

#ifndef EMP_MATH_COMBINEDBINOMIALDISTRIBUTION_HPP_INCLUDE
#define EMP_MATH_COMBINEDBINOMIALDISTRIBUTION_HPP_INCLUDE

#include "./Distribution.hpp"

namespace emp{
  /// \brief A collection of distributions that allows for pulls from a binomial distribution with arbitrary N while only storing log_2(N) distributions
  class CombinedBinomialDistribution{
    protected:
      emp::vector<Binomial> distribution_vec; /**< The collection of binomial distributions
                                                   used to construct any N */
      double p;             ///< The success probability of a single Bernoulli trial
      size_t cur_max_power; /**< The maximum power of two currently supported by our
                                 distributions */

      /// Fetch the smallest power of two that is larger than N
      size_t GetMaxPower(size_t n) const {
        size_t power = 0;
        for(size_t val = 1; val < n; val <<= 1, ++power){ ; }
        return power;
      }

    public:
      CombinedBinomialDistribution() : p(0), cur_max_power(0){ ; }
      CombinedBinomialDistribution(double _p, size_t _starting_n) : p(_p), cur_max_power(0){
        Expand(_starting_n);
      }

      /// Sample a binomial distribution with n events
      size_t PickRandom(size_t n, Random & random){
        size_t local_max_power = GetMaxPower(n);
        size_t result = 0;
        if(local_max_power > cur_max_power) Expand(n);
        for(size_t power = 0; power <= local_max_power; ++power){
          if( (n & (1 << power)) != 0){
            result += distribution_vec[power].PickRandom(random);
          }
        }
        return result;
      }

      /// Reset the distribution with a new probability, p, and a starting n value
      void Setup(double _p, size_t _n){
        distribution_vec.clear();
        cur_max_power = 0;
        p = _p;
        if(_n > (1ull << cur_max_power)) Expand(_n);
      }

      /// Create more distributions to handle the given value of n
      void Expand(size_t max_n){
        cur_max_power = GetMaxPower(max_n);
        for(size_t power = distribution_vec.size(); power <= cur_max_power; ++power){
          distribution_vec.emplace_back(p, 1 << power);
        }
      }

      /// Fetch the current maximum power handled by this combined distribution
      size_t GetCurMaxPower(){ return cur_max_power; }
  };
}

#endif // #ifndef EMP_MATH_COMBINEDBINOMIALDISTRIBUTION_HPP_INCLUDE
