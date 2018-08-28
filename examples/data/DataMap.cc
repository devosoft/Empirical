/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  DataMap.cc
 *  @brief An example file for using DataMaps
 */

#include <iostream>

#include "data/DataMap.h"

int main()
{
  emp::DataMap<std::string, bool, int, double> dmap;

  dmap.Add("fitness", 0.0);
  dmap.Add("do_muts", false);
  dmap.Add<std::string>("name", "MyOrg1");
  dmap.Add<std::string>("name2", "MyOrg2");
  dmap.Add<std::string>("name3", "MyOrg3");

  std::cout << "Testing!" << std::endl;
}
