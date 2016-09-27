//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include "../../tools/Random.h"
#include "../../tools/vector.h"
#include "../../tools/WeightedSet.h"

int main()
{
  const int SET_SIZE = 10000;

  emp::WeightedSet test_set(SET_SIZE);
  test_set[0] = 0.0;
  test_set[1] = 0.5;
  test_set[2] = 1.0;
  test_set[3] = 1.5;
  test_set[4] = 2.0;
  test_set[5] = 2.5;
  test_set[6] = 3.0;
  test_set[7] = 3.5;
  test_set[8] = 4.0;
  test_set[9] = 4.5;
  std::cout << "Ping!" << std::endl;

  emp::Random random;
  emp::vector<int> count(10,0);
  for (int i = 0; i < 45000; i++) {
    int id = test_set.Index(random.GetDouble(0,test_set.GetWeight()));
    count[id]++;
  }
  for (int id = 0; id < 10; id++) {
    std::cout << id << " : " << count[id] << std::endl;
  }

  std::cout << "Start weight = " << test_set.GetWeight() << std::endl;

  std::cout << "\n... adjusting set weights ...\n";
  for (int i = 0; i < 100000000; i++) {
    test_set[random.GetInt(SET_SIZE)] = random.GetDouble(0.0,5000.0);
  }

  std::cout << "End weight = " << test_set.GetWeight() << std::endl;

}
