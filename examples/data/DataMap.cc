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

  dmap.GetDefault<std::string>("name") = "FirstOrg";
  dmap.GetDefault<std::string>(1) = "Org TWO!";
  dmap.GetDefault<std::string>("name3") = "Test Output!";
  dmap.GetDefault<double>("fitness") = 1000000.1;

  std::cout << "Name 1 = " << dmap.GetDefault<std::string>("name") << std::endl;
  std::cout << "Name 2 = " << dmap.GetDefault<std::string>("name2") << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>("name3") << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>(2) << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>( dmap.GetID("name3") ) << std::endl;

  std::cout << "\nTypes:\n";
  std::cout << "fitness: " << dmap.GetType("fitness") << std::endl;
  std::cout << "do_muts: " << dmap.GetType("do_muts") << std::endl;
  std::cout << "name:    " << dmap.GetType("name") << std::endl;
  std::cout << "name2:   " << dmap.GetType("name2") << std::endl;
  std::cout << "name3:   " << dmap.GetType("name3") << std::endl;
}
