//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <ctime>
#include <iostream>
#include <string>
#include <unordered_set>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "emp/bits/BitVector.hpp"

int main()
{
  const size_t set_size = 100000;
  using TEST_TYPE = emp::BitVector;  // Change this to use std::vector<bool> instead.

  TEST_TYPE set1(set_size);
  TEST_TYPE set2(set_size);

  for (size_t i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  size_t total = 0;
  for (size_t i = 0; i < 10000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  double time = 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC;
  std::cout << "Time = " << time << std::endl;

  std::unordered_set<emp::BitVector> bv_set;
  bv_set.insert(set1);
  bv_set.insert(set2);

  emp::BitVector bv(10);
  std::cout << bv.Hash() << "  (initial, 10 bits)" << std::endl;
  bv[3] = true;
  std::cout << bv.Hash() << "  (bit 3 set to true)" << std::endl;
  bv.Resize(9);
  std::cout << bv.Hash() << "  (resized to 9.)" << std::endl;
  std::cout << bv.Hash() << "  (same as previous)" << std::endl;
  emp::BitVector bv2(12);
  std::cout << bv2.Hash() << "  (new bv with 12 bits)" << std::endl;
  bv2[3] = true;
  bv2[11] = true;
  std::cout << bv2.Hash() << "  (bits 3 and 11 set true)" << std::endl;
  bv2.Resize(9);
  std::cout << bv2.Hash() << "  (resized new bv to 9)" << std::endl;

}
