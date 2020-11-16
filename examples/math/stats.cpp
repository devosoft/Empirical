//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using vector_utils.h

#include <iostream>
#include <unordered_set>

#include "emp/base/vector.hpp"
#include "emp/math/stats.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  emp::vector<double> v1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
  const emp::vector<double> v2 = { 1.0, 5.0, 3.0, 4.0, 2.0, 6.0 };
  emp::vector<size_t> v3 = { 4, 6, 8, 10, 12, 14 };
  emp::vector<int> v4 = { -2, -1, 0, 1, 2, 4 };
  std::set<double> s1 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
  std::set<int> s4 = { -2, -1, 0, 1, 2, 3 };

  int a=-2, b=-1, c=0, d=1, e=2, f=3;
  emp::vector<int*> vp4 = { &a, &b, &c, &d, &e, &f };


  std::cout << "v1 = " << emp::to_string(v1) << std::endl;
  std::cout << "v2 = " << emp::to_string(v2) << std::endl;
  std::cout << "v3 = " << emp::to_string(v3) << std::endl;
  std::cout << "v4 = " << emp::to_string(v4) << std::endl;

  std::cout << std::endl;
  std::cout << "Sum(v1) = " << emp::Sum(v1)
            << "  Mean(v1) = " << emp::Mean(v1)
            << "  Var(v1) = " << emp::Variance(v1)
            << "  StdDev(v1) = " << emp::StandardDeviation(v1)
            << std::endl;
  std::cout << "Sum(v2) = " << emp::Sum(v2)
            << "  Mean(v2) = " << emp::Mean(v2)
            << "  Var(v2) = " << emp::Variance(v2)
            << "  StdDev(v2) = " << emp::StandardDeviation(v2)
            << std::endl;
  std::cout << "Sum(v3) = " << emp::Sum(v3)
            << "  Mean(v3) = " << emp::Mean(v3)
            << "  Var(v3) = " << emp::Variance(v3)
            << "  StdDev(v3) = " << emp::StandardDeviation(v3)
            << std::endl;
  std::cout << "Sum(v4) = " << emp::Sum(v4)
            << "  Mean(v4) = " << emp::Mean(v4)
            << "  Var(v4) = " << emp::Variance(v4)
            << "  StdDev(v4) = " << emp::StandardDeviation(v4)
            << std::endl;
  std::cout << "Sum(s1) = " << emp::Sum(s1)
            << "  Mean(s1) = " << emp::Mean(s1)
            << "  Var(s1) = " << emp::Variance(s1)
            << "  StdDev(s1) = " << emp::StandardDeviation(s1)
            << std::endl;
  std::cout << "Sum(s4) = " << emp::Sum(s4)
            << "  Mean(s4) = " << emp::Mean(s4)
            << "  Var(s4) = " << emp::Variance(s4)
            << "  StdDev(s4) = " << emp::StandardDeviation(s4)
            << std::endl;
  std::cout << "Sum(vp4) = " << emp::Sum(vp4)
            << "  Mean(vp4) = " << emp::Mean(vp4)
            << "  Var(vp4) = " << emp::Variance(vp4)
            << "  StdDev(vp4) = " << emp::StandardDeviation(vp4)
            << std::endl;

  std::cout << std::endl;
  emp::vector<char> v5 = { 'a', 'b', 'c', 'a', 'a', 'b', 'c', 'a' };
  char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'a';
  emp::vector<char*> vp5 = { &c1, &c2, &c3, &c4, &c1, &c2, &c3, &c4 };

  std::cout << "v5 = " << emp::to_string(v5) << std::endl;
  std::cout << "ShannonEntropy(v5) = " << emp::ShannonEntropy(v5) << std::endl;
  std::cout << "ShannonEntropy(vp5) = " << emp::ShannonEntropy(vp5) << std::endl;

}
