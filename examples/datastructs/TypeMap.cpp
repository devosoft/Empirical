/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file TypeMap.cpp
 *  @brief Some example code for using emp::BitSet
 */

#include <iostream>
#include <string>

#include "emp/datastructs/TypeMap.hpp"

int main()
{
  std::cout << "TypeMap Example.\n";

  emp::TypeMap<std::string> type_map;

  type_map.Get<bool>() = "This is a bool.";
  type_map.Get<double>() = "This is a double.";
  type_map.Get<int>() = "This is an int.";
  type_map.Get<std::string>() = "This is an std::string.";

  std::cout << "Bool message: " << type_map.Get<bool>() << std::endl;
  std::cout << "Double message: " << type_map.Get<double>() << std::endl;
  std::cout << "Int message: " << type_map.Get<int>() << std::endl;
  std::cout << "String message: " << type_map.Get<std::string>() << std::endl;
}
