//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using tuple_utils.h

#include <iostream>
#include <unordered_map>

#include "../../tools/tuple_utils.h"

int main()
{
  std::cout << "Ping!" << std::endl;

  std::unordered_map<std::tuple<int,int,int>, double, emp::TupleHash<int,int,int>> test_map;

  std::tuple<int,int,int> x = std::make_tuple(1,2,3);
  test_map[x] = 1.5;

}
