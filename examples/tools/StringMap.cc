//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::StringMap and emp::StringID

#include <iostream>
#include "tools/StringMap.h"

#define PRINT_VAL(X) std::cout << #X " = " << X << std::endl

int main()
{
  emp::StringMap<int> test_map;

  test_map["Ten"] = 10;
  test_map["Zero"] = 0;
  test_map["NegTen"] = -10;

  test_map["twenty"] = 20;

  PRINT_VAL( test_map["Ten"] );
}
