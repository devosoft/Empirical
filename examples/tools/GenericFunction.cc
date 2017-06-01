//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::GenericFunction

#include <iostream>

#include "../../tools/GenericFunction.h"

int main()
{
  emp::Function<int(int,int)> fun( [](int x, int y){ return x+y; } );
  emp::Function<int(int,int)> fun2;

  std::cout << fun(2,3) << std::endl;
}

