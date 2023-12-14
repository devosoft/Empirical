/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016
*/
/**
 *  @file
 *  @brief Some examples code for using emp::Random
 */

#include <iostream>
#include <vector>

#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

int main()
{
  emp::Random random;

  std::cout << "Digits in random orders:" << std::endl;

  size_t num_samples = 10;

  std::cout << "Permutations: " << std::endl;
  for (size_t s  = 0; s < num_samples; s++) {
    emp::vector<size_t> permut = emp::GetPermutation(random, 10);
    for (size_t i = 0; i < 10; i++) {
      std::cout << permut[i];
    }
    std::cout << std::endl;
  }

  std::cout << "Exponentials: " << std::endl;
  double p = 0.5;
  std::cout << "p = " << p << std::endl;
  for (size_t s  = 0; s < num_samples; s++) std::cout << random.GetExponential(p) << std::endl;

  p = 0.1;
  std::cout << "\np = " << p << std::endl;
  for (size_t s  = 0; s < num_samples; s++) std::cout << random.GetExponential(p) << std::endl;

  p = 0.9;
  std::cout << "\np = " << p << std::endl;
  for (size_t s  = 0; s < num_samples; s++) std::cout << random.GetExponential(p) << std::endl;
}
