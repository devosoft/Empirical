//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

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
  std::cout << "Ping!" << '\n';

  // Test AdaptTepmplate()
  emp::AdaptTemplate_Arg1< Temp2Type<std::string, std::string>, double > map2;

  map2.a = 3.45;
  map2.b = "abcd";

  std::cout << map2.a << '\n';
  std::cout << map2.b << '\n';


  // Test CombineHash()
  std::cout << "\nHash results...:\n";
  std::cout << "hash(2) = " << std::hash<int>()(2) << '\n'
	    << "hash(3) = " << std::hash<int>()(3) << '\n'
	    << "hash(4) = " << std::hash<int>()(4) << '\n'
	    << "CombineHash(4) = " << emp::CombineHash(4) << '\n'
	    << "CombineHash(2,3) = " << emp::CombineHash(2,3) << '\n'
	    << "CombineHash(2,3) = " << emp::CombineHash(2,3) << '\n'
	    << "CombineHash(3,2) = " << emp::CombineHash(3,2) << '\n'
	    << "CombineHash(3,4) = " << emp::CombineHash(3,4) << '\n'
	    << "CombineHash(2,3,4) = " << emp::CombineHash(2,3,4) << '\n';

  // Test Math...
  using math_t = emp::tIntMath<1, 2, 3, 4>;
  std::cout << "\nMath Tests:\n";
  std::cout << "Sum = " << math_t::Sum() << '\n';
  std::cout << "Prod = " << math_t::Product() << '\n';

  std::cout << "\nTruncateCall Tests:\n";
  std::function<int(int,int,int)> fun(Sum3);
  std:: cout << "emp::TruncateCall(fun, 1,2,3,4,5) = "
	     << emp::TruncateCall(fun, 1,2,3,4,5)
	     << '\n';

  std::cout << "\nAdaptFunction Tests:\n";
  auto fun2 = emp::AdaptFunction<int,int,int,int>::Expand<double, char>(fun);
  std::cout << "expanded fun = " << fun2(100,200,300,1.234,'a') << '\n';

  // Test type tests...
  std::cout << "\nTest type examples:\n";
  std::cout << "std::is_integral<int> value = "
	    << emp::test_type_value<std::is_integral, int>()
	    << '\n';
  std::cout << "std::is_integral<double> value = "
	    << emp::test_type_value<std::is_integral, double>()
	    << '\n';

  std::cout << "std::is_integral<int> exists = "
	    << emp::test_type_exist<std::is_integral, int>()
	    << '\n';
  std::cout << "std::is_integral<double> exists = "
	    << emp::test_type_exist<std::is_integral, double>()
	    << '\n';

}
