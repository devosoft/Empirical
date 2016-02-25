//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples of code using reflection techniques.

#include <iostream>

#include "../../tools/reflection.h"

struct A {
  using test_type = double;

  static const int X = 1;
};

struct B {
  static const int X = 2;
};

template <typename T>
struct Wrapper {
  EMP_CREATE_TYPE_FALLBACK(new_type, T, test_type, int);

  static constexpr new_type VALUE = 5;
};

int main()
{
  Wrapper<A> wrap_A;
  Wrapper<B> wrap_B;

  std::cout << Wrapper<A>::VALUE/2 << std::endl;
  std::cout << Wrapper<B>::VALUE/2 << std::endl;
}
