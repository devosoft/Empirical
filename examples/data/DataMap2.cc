/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  DataMap.cc
 *  @brief An example file for using DataMaps
 */

#include <iostream>

#include "data/DataMap2.h"

int main()
{
  emp::DataMap<emp::MemoryVector> dmap;

  size_t id1 = dmap.Add<double>("fitness", 0.0);
  size_t id2 = dmap.Add<bool>("do_muts", false);
  size_t id3 = dmap.Add<std::string>("name", "MyOrg1");
  size_t id4 = dmap.Add<std::string>("name2", "MyOrg2");
  size_t id5 = dmap.Add<std::string>("name3", "MyOrg3");

  std::cout << "IDs: "
            << " id1 = " << id1
            << " id2 = " << id2
            << " id3 = " << id3
            << " id4 = " << id4
            << " id5 = " << id5
            << std::endl;

  dmap.GetDefault<std::string>("name") = "FirstOrg";
  dmap.GetDefault<std::string>(id4) = "Org TWO!";
  dmap.GetDefault<std::string>("name3") = "Test Output!";
  dmap.GetDefault<double>("fitness") = 1000000.1;

  std::cout << "Name 1 = " << dmap.GetDefault<std::string>("name") << std::endl;
  std::cout << "Name 2 = " << dmap.GetDefault<std::string>("name2") << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>("name3") << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>(id5) << std::endl;
  std::cout << "Name 3 = " << dmap.GetDefault<std::string>( dmap.GetID("name3") ) << std::endl;

  std::cout << "\nTypes:\n";
  std::cout << "fitness: " << dmap.GetType("fitness") << std::endl;
  std::cout << "do_muts: " << dmap.GetType("do_muts") << std::endl;
  std::cout << "name:    " << dmap.GetType("name") << std::endl;
  std::cout << "name2:   " << dmap.GetType("name2") << std::endl;
  std::cout << "name3:   " << dmap.GetType("name3") << std::endl;
}
