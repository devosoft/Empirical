/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  VarMap.cc
 *  @brief An example file for using VarMap
 */

#include <iostream>

#include "emp/data/VarMap.hpp"

int main()
{
  emp::VarMap vmap;

  vmap.Add("fitness", 0.0);
  vmap.Add("do_muts", false);
  vmap.Add<std::string>("name", "MyOrg1");
  vmap.Add<std::string>("name2", "MyOrg2");
  vmap.Add<std::string>("name3", "MyOrg3");

  vmap.Get<std::string>("name") = "FirstOrg";
  vmap.Get<std::string>(3) = "Org TWO!";
  vmap.Get<std::string>("name3") = "Test Output!";
  vmap.Get<double>("fitness") = 1000000.1;

  std::cout << "Name 1 = " << vmap.Get<std::string>("name") << '\n';
  std::cout << "Name 2 = " << vmap.Get<std::string>("name2") << '\n';
  std::cout << "Name 3 = " << vmap.Get<std::string>("name3") << '\n';
  std::cout << "Name 3 = " << vmap.Get<std::string>(4) << '\n';
  std::cout << "Name 3 = " << vmap.Get<std::string>( vmap.GetID("name3") ) << '\n';

  // Try modifying variable references in VarMap.

  int x = 5;
  vmap.Add<int &>("test_int", x);
  vmap.Get<int &>("test_int") += 10;
  std::cout << "x = " << x << '\n';

//   std::cout << "\nTypes:\n";
//   std::cout << "fitness: " << vmap.GetType("fitness") << '\n';
//   std::cout << "do_muts: " << vmap.GetType("do_muts") << '\n';
//   std::cout << "name:    " << vmap.GetType("name") << '\n';
//   std::cout << "name2:   " << vmap.GetType("name2") << '\n';
//   std::cout << "name3:   " << vmap.GetType("name3") << '\n';
}
