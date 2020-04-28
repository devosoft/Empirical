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

  int a=-2, b=-1, c=0, d=1, e=2, f=3;
  emp::vector<int*> vp4 = { &a, &b, &c, &d, &e, &f };

  
  std::cout << "v1 = " << emp::to_string(v1) << std::endl;
  std::cout << "v2 = " << emp::to_string(v2) << std::endl;
  std::cout << "v3 = " << emp::to_string(v3) << std::endl;
  std::cout << "v4 = " << emp::to_string(v4) << std::endl;

  std::cout << std::endl;
  std::cout << "Sum(v1) = " << emp::Sum(v1) << std::endl;
  std::cout << "Sum(v2) = " << emp::Sum(v2) << std::endl;
  std::cout << "Sum(v3) = " << emp::Sum(v3) << std::endl;
  std::cout << "Sum(v4) = " << emp::Sum(v4) << std::endl;
  std::cout << "Sum(s1) = " << emp::Sum(s1) << std::endl;
  std::cout << "Sum(s4) = " << emp::Sum(s4) << std::endl;
  std::cout << "Sum(vp4) = " << emp::Sum(vp4) << std::endl;

  std::cout << std::endl;
  emp::vector<char> v5 = { 'a', 'b', 'c', 'a', 'a', 'b', 'c', 'a' };
  char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'a';
  emp::vector<char*> vp5 = { &c1, &c2, &c3, &c4, &c1, &c2, &c3, &c4 };

  std::cout << "v5 = " << emp::to_string(v5) << std::endl;
  std::cout << "ShannonEntropy(v5) = " << emp::ShannonEntropy(v5) << std::endl;
  std::cout << "ShannonEntropy(vp5) = " << emp::ShannonEntropy(vp5) << std::endl;
}
