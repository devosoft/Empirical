/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file meta.cpp
 */

#include <bitset>
#include <iostream>
#include <map>
#include <string>

#include "emp/meta/meta.hpp"

template <typename A, typename B>
struct Temp2Type {
  A a;
  B b;
  Temp2Type() : a(), b() { ; }
};

int Sum3(int x, int y, int z) { return x+y+z; }

int main()
{
  std::cout << "Ping!" << std::endl;

  // Test AdaptTepmplate()
  emp::AdaptTemplate_Arg1< Temp2Type<std::string, std::string>, double > map2;

  map2.a = 3.45;
  map2.b = "abcd";

  std::cout << map2.a << std::endl;
  std::cout << map2.b << std::endl;


  // Test Math...
  using math_t = emp::tIntMath<1, 2, 3, 4>;
  std::cout << "\nMath Tests:\n";
  std::cout << "Sum = " << math_t::Sum() << std::endl;
  std::cout << "Prod = " << math_t::Product() << std::endl;

  std::cout << "\nTruncateCall Tests:\n";
  std::function<int(int,int,int)> fun(Sum3);
  std:: cout << "emp::TruncateCall(fun, 1,2,3,4,5) = "
       << emp::TruncateCall(fun, 1,2,3,4,5)
       << std::endl;

  std::cout << "\nAdaptFunction Tests:\n";
  auto fun2 = emp::AdaptFunction<int,int,int,int>::Expand<double, char>(fun);
  std::cout << "expanded fun = " << fun2(100,200,300,1.234,'a') << std::endl;

  // Test type tests...
  std::cout << "\nTest type examples:\n";
  std::cout << "std::is_integral<int> value = "
      << emp::test_type_value<std::is_integral, int>()
      << std::endl;
  std::cout << "std::is_integral<double> value = "
      << emp::test_type_value<std::is_integral, double>()
      << std::endl;

  std::cout << "std::is_integral<int> exists = "
      << emp::test_type_exist<std::is_integral, int>()
      << std::endl;
  std::cout << "std::is_integral<double> exists = "
      << emp::test_type_exist<std::is_integral, double>()
      << std::endl;

}
