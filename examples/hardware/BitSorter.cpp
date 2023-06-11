/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file BitSorter.cpp
 *  @brief This is example code for using BitSorter.h
 */

#include <iostream>

#include "emp/hardware/BitSorter.hpp"
#include "emp/math/Random.hpp"

int main()
{
  std::cout << "Test." << std::endl;

  emp::BitSorter test_sorter;

  emp::Random random;
  for (size_t i = 0; i < 60; i++) {
    test_sorter.AddCompare(random.GetUInt(16), random.GetUInt(16));
  }

  size_t vals = 11110;
  std::cout << "Pre-sort Vals  = " << test_sorter.ToString(vals) << std::endl;
  vals = test_sorter.Sort(vals);
  std::cout << "Post-sort Vals = " << test_sorter.ToString(vals) << std::endl;
  std::cout << "Size = " << test_sorter.GetSize() << std::endl;
  std::cout << "Depth = " << test_sorter.CalcDepth() << std::endl;

  std::cout << "Sort count = " << test_sorter.CountSortable() << "/" << (1 << 16) << std::endl;
}
