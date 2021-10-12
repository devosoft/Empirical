//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataNode demonstrating how to track different types of data.

#include <iostream>

#include "emp/data/DataManager.hpp"

using data = emp::data;

int main()
{
  emp::DataManager<double, data::Current, data::Range, data::Pull, data::Log> dataM;

  dataM.New("fitness");
  dataM.New("merit");
  dataM.New("fidelity");

  dataM.AddData("fitness", 3.0);
  dataM.Get("fitness").Add(6.5);
  auto & fit_node = dataM.Get("fitness");
  fit_node.Add(10.0);

  auto & merit_node = dataM.Get("merit");
  merit_node.Add(300, 650, 500);

  dataM.AddData("fidelity", 1.0, 0.8, 0.9);

  for (auto & x : dataM.GetNodes()) {
    auto & node = *(x.second);
    std::cout << x.first
              << " cur=" << node.GetCurrent()
              << " ave=" << node.GetMean()
              << " min=" << node.GetMin()
              << " max=" << node.GetMax()
              << " tot=" << node.GetTotal()
              << std::endl;
  }
}
