//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using tuple_utils.h

#include <iostream>
#include <unordered_map>

#include "tools/tuple_utils.h"

int Sum3(int x, int y, int z) { return x+y+z; }

void PrintTwice(int x) { std::cout << x << x << std::endl; }

int main()
{
  std::cout << "Ping!" << std::endl;

  std::unordered_map<std::tuple<int,int,int>, double, emp::TupleHash<int,int,int>> test_map;

  std::tuple<int,int,int> tup = std::make_tuple(1,2,3);
  test_map[tup] = 1.5;

  // Use ApplyTuple
  std::cout << "\nApplyTuple results...:\n";
  int x = 10;
  int y = 13;
  int z = 22;
  auto test_tup = std::make_tuple(x,y,z);
  std::cout << "Sum3(" << x << "," << y << "," << z << ") = "
	    << emp::ApplyTuple(Sum3, test_tup) << std::endl;
  std::cout << "Prod3(" << x << "," << y << "," << z << ") = "
	    << emp::ApplyTuple([](int x, int y, int z){ return x*y*z; }, test_tup) << std::endl;

  std::cout << "CombineHash(" << x << "," << y << "," << z << ") = "
      << emp::ApplyTuple(emp::CombineHash<int,int,int>, test_tup) << std::endl;

  emp::TupleIterate(tup, PrintTwice);
}
