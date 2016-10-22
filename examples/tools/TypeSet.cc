//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using TypeSet

#include <iostream>
#include <string>

#include "../../tools/TypeSet.h"

int main()
{
  std::cout << "Testing!" << std::endl;

  using test_t = emp::TypeSet<int, std::string, float, bool, double>;
  std::cout << "Num types = " << test_t::GetSize() << std::endl;
  std::cout << "float pos = " << test_t::GetID<float>() << std::endl;

  std::cout << "Num types (with add) = " << test_t::add_t<long long>::GetSize() << std::endl;
  std::cout << "float pos (with pop) = " << test_t::pop_front_t::GetID<float>() << std::endl;

  using test2_t = emp::TypeSet<int, int, int, int>;
  using test3_t = emp::TypeSet<uint64_t>;

  std::cout << std::endl;
  std::cout << "Num test_t types:  " << test_t::GetSize() << std::endl;
  std::cout << "Num test2_t types: " << test2_t::GetSize() << std::endl;
  std::cout << "Num test3_t types: " << test3_t::GetSize() << std::endl;
}
