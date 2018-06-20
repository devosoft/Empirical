//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Binomial

#include <iostream>
#include "tools/Random.h"
#include "tools/Binomial.h"

int main()
{
  emp::Random random;

  emp::Binomial bi1000(0.003, 1000);

  // Print out the first values in the distribution.
  for (size_t i = 0; i < 20; i++) {
    std::cout << "bi1000[" << i << "] = " << bi1000[i] << "\n";
  }
  std::cout << "Total = " << bi1000.GetTotalProb() << std::endl;

  // Pick some random values...
  std::cout << "\nSome random values:";
  for (size_t i = 0; i < 100; i++) {
    std::cout << " " << bi1000.PickRandom(random);
  }
  std::cout << std::endl;
}
