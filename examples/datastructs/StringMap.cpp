//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::StringMap and emp::StringID

#include <iostream>
#include "emp/datastructs/StringMap.hpp"

#define PRINT_VAL(X) std::cout << #X " = " << X << std::endl

int main()
{
  emp::StringMap<int> test_map;

  test_map["Ten"] = 10;
  test_map["Five"] = 5;
  test_map["Zero"] = 0;
  test_map["NegTen"] = -10;

  test_map["Fifteen"] = test_map["Ten"] + test_map["Five"];
  test_map["Twenty"] = test_map["Ten"] * 2;

  PRINT_VAL( test_map["Fifteen"] );

  test_map[EMP_STRING("Alpha")] = 1;
  test_map[EMP_STRING("Beta")] = 2;
  test_map[EMP_STRING("Gamma")] = 3;

  PRINT_VAL(test_map[EMP_STRING("Beta")]);
  PRINT_VAL(test_map["Beta"]);

  test_map.CTGet("Alpha") = 5;
  test_map.CTGet("Beta") = 6;
  test_map.CTGet("Gamma") = 7;

  PRINT_VAL(test_map[EMP_STRING("Gamma")]);
  PRINT_VAL(test_map["Gamma"]);
 }
