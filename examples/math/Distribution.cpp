//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018-2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Distribution are derived classes.

#include <iostream>
#include "emp/math/Random.hpp"
#include "emp/math/Distribution.hpp"

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

  // And total some more random picks (to take a bit of time).
  size_t total = 0;
  const size_t test_count = 10000000;

  for (size_t i = 0; i < test_count; i++) {
    total += bi1000.PickRandom(random);
  }

  std::cout << "Average of " << test_count << " = "
	    << (((double) total) / (double) test_count)
	    << std::endl;


  //emp::NegativeBinomial nbi10(0.5, 2);
  emp::NegativeBinomial nbi10(0.3, 10);

  std::cout << "\n-- Negative Binomial--\n";

  std::cout << "size = " << nbi10.GetSize() << std::endl
	    << "total_prob = " << nbi10.GetTotalProb() << std::endl;

  // for (size_t i = 0; i < 10; i++) {
  for (size_t i = 9; i < 40; i++) {
    std::cout << "nbi10[" << i << "] = " << nbi10[i] << "\n";
  }
}
