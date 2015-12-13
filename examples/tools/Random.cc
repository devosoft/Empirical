//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include <vector>
#include "../../tools/Random.h"

int main()
{
  emp::Random random;
  
  std::cout << "Digits in random orders:" << std::endl;

  int num_samples = 10;
  for (int s  = 0; s < num_samples; s++) {
    std::vector<int> permut = random.GetPermutation(10);
    for (int i = 0; i < 10; i++) {
      std::cout << permut[i];
    }
    std::cout << std::endl;
  }
}
