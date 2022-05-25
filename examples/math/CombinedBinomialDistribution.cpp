/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022
 *
 *  @file CombinedBinomialDistribution.cpp
 *  @brief Some examples code for using emp::CombinedBinomialDistribution
 */


#include "emp/math/CombinedBinomialDistribution.hpp"
#include "emp/math/Random.hpp"

int main(int argc, char* argv[])
{
  if(argc != 4){
    std::cout << "Error! Expecting exactly three command line arguments: " 
              << "p n num_trials" << std::endl;
    emp_assert(false);
  }
  double p = std::stod(argv[1]);
  size_t n = std::stoi(argv[2]);
  size_t num_trials = std::stoi(argv[3]);
  
  emp::Random random;
  emp::CombinedBinomialDistribution distribution(p, 1);

  double mean = 0;

  for(size_t i = 0; i < num_trials; i++){
    mean += (double)distribution.PickRandom(n, random) / num_trials;
  }
  std::cout << "Mean after " << num_trials << " trials: " << mean << std::endl;
  return 0;
}
