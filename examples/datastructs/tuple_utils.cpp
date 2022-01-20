/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file tuple_utils.cpp
 *  @brief Some examples code for using tuple_utils.hpp
 */

#include <iostream>
#include <unordered_map>

#include "emp/datastructs/tuple_utils.hpp"

int Sum3(int x, int y, int z) { return x+y+z; }

void PrintTwice(int x) { std::cout << x << x << std::endl; }
void PrintProduct(int x, int y) { std::cout << (x * y) << std::endl; }

int main()
{
  std::cout << "Ping!" << std::endl;

  std::unordered_map<std::tuple<int,int,int>, double, emp::TupleHash<int,int,int>> test_map;

  std::tuple<int,int,int> tup = std::make_tuple(1,2,3);
  test_map[tup] = 1.5;

  std::cout << "\nPrintTwice with TupleIterate:\n";
  emp::TupleIterate(tup, PrintTwice);

  std::cout << "\nPrint *3, *4, *5 using a 2-input TupleIterate:\n";
  std::tuple<int,int,int> mult_tup(3,4,5);
  emp::TupleIterate(tup, mult_tup, PrintProduct);
}
