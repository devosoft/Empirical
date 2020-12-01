//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::GenericFunction

#include <iostream>

#include "emp/functional/GenericFunction.hpp"

int main()
{
  emp::Function<int(int,int)> fun( [](int x, int y){ return x+y; } );
  emp::Function<int(int,int)> fun2;
  emp::GenericFunction * fun_g = &fun;
  emp::GenericFunction * fun_g2 = new emp::Function<void(double val)>([](double val){
    std::cout << "2*Val = " << (2*val) << std::endl;
  });

  std::cout << fun(2,3) << std::endl;
  std::cout << fun_g->Call<int, int,int>(10,12) << std::endl;
  fun_g2->Call<void, double>(3.14);
  fun_g2->Call<void>(20.20);
  fun_g2->Convert<void(double)>()->Call(11.11);
}
