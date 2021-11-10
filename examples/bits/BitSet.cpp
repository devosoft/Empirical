/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file BitSet.cpp
 *  @brief Some example code for using emp::BitSet
 */

#include <ctime>
#include <iostream>
#include <string>
#include <unordered_set>

#include "emp/bits/BitSet.hpp"
#include "emp/math/Random.hpp"

constexpr int set_size = 100000;

double BitSetMath() {
  using TEST_TYPE = emp::BitSet<set_size>;

  TEST_TYPE set1, set2;

  for (size_t i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  double total = 0.0;
  for (int i = 0; i < set_size; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += (double) set5.CountOnes();
  }

  return total;
}

int main()
{
  using TEST_TYPE = emp::BitSet<set_size>;

  TEST_TYPE set1, set2;

  for (size_t i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  double total = 0.0;
  for (size_t i = 0; i < set_size; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += (double) set5.CountOnes();
  }

  std::cout << "total = " << total << std::endl;
  double bsm = BitSetMath();
  std::cout << "BitSetMath() = " << bsm << std::endl;

//   emp::BitVector bv(10);
//   std::cout << bv.Hash() << "  (initial, 10 bits)" << std::endl;
//   bv[3] = true;
//   std::cout << bv.Hash() << "  (bit 3 set to true)" << std::endl;
//   bv.Resize(9);
//   std::cout << bv.Hash() << "  (resized to 9.)" << std::endl;
//   std::cout << bv.Hash() << "  (same as previous)" << std::endl;
//   emp::BitVector bv2(12);
//   std::cout << bv2.Hash() << "  (new bv with 12 bits)" << std::endl;
//   bv2[3] = true;
//   bv2[11] = true;
//   std::cout << bv2.Hash() << "  (bits 3 and 11 set true)" << std::endl;
//   bv2.Resize(9);
//   std::cout << bv2.Hash() << "  (resized new bv to 9)" << std::endl;

  emp::Random random;
  std::cout << "Printing random 5-bit BitSets..." << std::endl;
  for (size_t i = 0; i < 10; i++) {
    emp::BitSet<5> print_set(random);
    std::cout << print_set << " : " << print_set.GetUInt(0) << std::endl;
  }
}
