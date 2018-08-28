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

  dmap.Get<std::string>("name") = "FirstOrg";
  dmap.Get<std::string>(1) = "Org TWO!";
  dmap.Get<std::string>("name3") = "Test Output!";
  dmap.Get<double>("fitness") = 1000000.1;

  std::cout << "Name 1 = " << dmap.Get<std::string>("name") << std::endl;
  std::cout << "Name 2 = " << dmap.Get<std::string>("name2") << std::endl;
  std::cout << "Name 3 = " << dmap.Get<std::string>("name3") << std::endl;
  std::cout << "Name 3 = " << dmap.Get<std::string>(2) << std::endl;

}
