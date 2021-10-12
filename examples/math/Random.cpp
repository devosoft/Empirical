//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include <vector>
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

int main()
{
  emp::Random random;

  std::cout << "Digits in random orders:" << std::endl;

  size_t num_samples = 10;
  for (size_t s  = 0; s < num_samples; s++) {
    emp::vector<size_t> permut = emp::GetPermutation(random, 10);
    for (size_t i = 0; i < 10; i++) {
      std::cout << permut[i];
    }
    std::cout << std::endl;
  }
}
