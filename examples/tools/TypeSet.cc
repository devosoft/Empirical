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

  std::cout << "Num types (with add) = " << test_t::AddType<long long>::GetSize() << std::endl;
}
