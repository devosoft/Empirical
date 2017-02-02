//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataFile demonstrating how to generate

#include <iostream>

#include "../../data/DataFile.h"

using data = emp::data;

int main()
{
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

  double frac = 0.0;
  for (size_t i = 0; i < 10; i++) {
    data_fracs.Add(frac += 0.01);
    data_squares.Add((int)i*i);
    data_cubes.Add(i*i*i);
    dfile.Update();

    std::cout << i << std::endl;
  }

  dfile.SetupLine("[[",":", "]]\n");
  for (size_t i = 10; i < 20; i++) {
    data_fracs.Add(frac += 0.01);
    data_squares.Add((int)i*i);
    data_cubes.Add(i*i*i);
    dfile.Update();

    std::cout << i << std::endl;
  }
}
