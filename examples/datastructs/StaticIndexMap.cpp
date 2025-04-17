/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/datastructs/StaticIndexMap.hpp"
#include "emp/math/Random.hpp"

int main()
{
  emp::Random random;

  emp::vector<double> test_weights(10);
  test_weights[0] = 0.0;
  test_weights[1] = 0.5;
  test_weights[2] = 1.0;
  test_weights[3] = 1.5;
  test_weights[4] = 2.0;
  test_weights[5] = 2.5;
  test_weights[6] = 3.0;
  test_weights[7] = 3.5;
  test_weights[8] = 4.0;
  test_weights[9] = 4.5;
  emp::StaticIndexMap sim(test_weights);

  emp::vector<int> count((size_t)10,(int)0);
  for (size_t i = 0; i < 45000; i++) {
    size_t id = sim.Index(random.GetDouble(0,sim.GetWeight()));
    count[id]++;
  }
  std::cout << "Pick counts:\n";
  for (size_t id = 0; id < 10; id++) {
    std::cout << id << " : " << count[id] << std::endl;
  }

  std::cout << "Total weight = " << sim.GetWeight() << std::endl;

}
