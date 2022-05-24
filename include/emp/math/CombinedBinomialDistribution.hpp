/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
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

#include "./Distribution.hpp"

namespace emp{
  class CombinedBinomialDistribution{
    protected:
      emp::vector<Binomial> distribution_vec;
      double p;
      size_t cur_max_power;

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

      /// Pick an item from a distribution using a value between 0.0 and 1.0.
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

      void Setup(double _p, size_t _n){
        distribution_vec.clear();
        cur_max_power = 0;
        p = _p; 
        if(_n > (1ull << cur_max_power)) Expand(_n);
      }

      void Expand(size_t max_n){
        cur_max_power = GetMaxPower(max_n);
        for(size_t power = distribution_vec.size(); power <= cur_max_power; ++power){
          distribution_vec.emplace_back(p, 1 << power);
        }
      }

      size_t GetCurMaxPower(){ return cur_max_power; }
  };
}
