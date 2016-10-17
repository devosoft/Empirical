//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <bitset>
#include <iostream>
#include <map>
#include <string>

#include "../../tools/meta.h"

template <typename A, typename B>
struct Temp2Type {
  A a;
  B b;
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


  // Test CombineHash()
  std::cout << "\nHash results...:\n";
  std::cout << "hash(2) = " << std::hash<int>()(2) << std::endl
	    << "hash(3) = " << std::hash<int>()(3) << std::endl
	    << "hash(4) = " << std::hash<int>()(4) << std::endl
	    << "CombineHash(4) = " << emp::CombineHash(4) << std::endl
	    << "CombineHash(2,3) = " << emp::CombineHash(2,3) << std::endl
	    << "CombineHash(2,3) = " << emp::CombineHash(2,3) << std::endl
	    << "CombineHash(3,2) = " << emp::CombineHash(3,2) << std::endl
	    << "CombineHash(3,4) = " << emp::CombineHash(3,4) << std::endl
	    << "CombineHash(2,3,4) = " << emp::CombineHash(2,3,4) << std::endl;

  // Test ApplyTuple
  std::cout << "\nApplyTuple results...:\n";
  int x = 10;
  int y = 13;
  int z = 22;
  auto test_tup = std::make_tuple(x,y,z);
  std::cout << "Sum3(" << x << "," << y << "," << z << ") = "
	    << emp::ApplyTuple(Sum3, test_tup) << std::endl;

  std::cout << "CombineHash(" << x << "," << y << "," << z << ") = "
      << emp::ApplyTuple(emp::CombineHash<int,int,int>, test_tup) << std::endl;

  // Test Math...
  using math_t = emp::tIntMath<1, 2, 3, 4>;
  std::cout << "\nMath Tests:\n";
  std::cout << "Sum = " << math_t::Sum() << std::endl;
  std::cout << "Prod = " << math_t::Product() << std::endl;
}
