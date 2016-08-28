//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <ctime>
#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#define EMP_TRACK_MEM

#include "../../tools/BitVector.h"

int main()
{
  const int set_size = 100000;
  using TEST_TYPE = emp::BitVector;  // Change this to use std::vector<bool> instead.

  TEST_TYPE set1(set_size);
  TEST_TYPE set2(set_size);

  for (int i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  int total = 0;
  for (int i = 0; i < 100000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;

  double time = 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC;
  //REQUIRE(time < 9000); // NOTE: WILL VARY INTENSELY ON VARIOUS SYSTEMS
  // SHOULD PROBABLY CHANGE

  std::cout << "Time = " << time;
}

