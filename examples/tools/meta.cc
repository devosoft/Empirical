//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <bitset>
#include <iostream>
#include <map>
#include <string>

#include "../../tools/functions.h"

template <typename A, typename B>
struct Temp2Type {
  A a;
  B b;
};

int main()
{
  std::cout << "Ping!" << std::endl;

  emp::AdaptTemplate_Arg1< Temp2Type<std::string, std::string>, double > map2;

  map2.a = 3.45;
  map2.b = "abcd";

  std::cout << map2.a << std::endl;
  std::cout << map2.b << std::endl;
}
