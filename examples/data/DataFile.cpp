/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file DataFile.cpp
 *  @brief Examples for DataFile demonstrating how to generate
 */

#include <iostream>

#include "emp/data/DataFile.hpp"

using data = emp::data;

int test_fun() {
  static int val = 10;
  return val += 3;
}

int main()
{
  int test_int = 5;

  emp::DataFile dfile("test_file.dat");

  emp::DataMonitor<double> data_fracs;
  emp::DataMonitor<int> data_squares;
  emp::DataMonitor<uint64_t> data_cubes;

  dfile.AddCurrent(data_fracs);
  dfile.AddCurrent(data_squares);
  dfile.AddCurrent(data_cubes);
  dfile.AddMean(data_cubes);
  dfile.AddTotal(data_cubes);
  dfile.AddMin(data_cubes);
  dfile.AddMax(data_cubes);
  dfile.AddFun<int>(test_fun);
  dfile.AddVar<int>(test_int);

  double frac = 0.0;
  for (size_t i = 0; i < 10; i++) {
    test_int += (int)i;
    data_fracs.Add(frac += 0.01);
    data_squares.Add((int)(i*i));
    data_cubes.Add(i*i*i);
    dfile.Update();

    std::cout << i << std::endl;
  }

  dfile.SetupLine("[[",":", "]]\n");
  for (size_t i = 10; i < 20; i++) {
    data_fracs.Add(frac += 0.01);
    data_squares.Add((int)(i*i));
    data_cubes.Add(i*i*i);
    dfile.Update();

    std::cout << i << std::endl;
  }
}
