/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file AnyFunction.cpp
 *  @brief Some example code for using emp::AnyFunction
 */

#include <iostream>

#include "emp/functional/AnyFunction.hpp"

int main()
{
  std::function<int(int,int)> sfun = [](int x, int y){ return x+y; };
  emp::AnyFunction fun( sfun );
  emp::AnyFunction fun2;
  emp::AnyFunction fun3;
  fun3.Set<void(double)>( [](double val) {
      std::cout << "2*Val = " << (2*val) << std::endl;
    });

  std::cout << fun.Call<int>(2,3) << std::endl;
  std::cout << fun.Call<int, int,int>(10,12) << std::endl;
  fun3.Call<void, double>(3.14);
  fun3.Call<void>(20.20);
  fun3.Call(11.11);
}
