//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using vector_utils.h

#include <iostream>
#include <unordered_set>

#include "base/vector.h"
#include "tools/stats.h"
#include "tools/string_utils.h"

int main()
{
  emp::vector<double> v1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
  emp::vector<double> v2 = { 1.0, 5.0, 3.0, 4.0, 2.0, 6.0 };
  emp::vector<size_t> v3 = { 7, 8, 9, 10, 11, 12 };
  emp::vector<int> v4 = { -2, -1, 0, 1, 2, 3 };
  std::set<double> s1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
  std::set<int> s4 = { -2, -1, 0, 1, 2, 3 };
  
  std::cout << "v1 = " << emp::to_string(v1) << std::endl;
  std::cout << "v2 = " << emp::to_string(v2) << std::endl;
  std::cout << "v3 = " << emp::to_string(v3) << std::endl;
  std::cout << "v4 = " << emp::to_string(v4) << std::endl;

}
