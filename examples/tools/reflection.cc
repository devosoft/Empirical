//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples of code using reflection techniques.

#include <iostream>

#include "../../tools/reflection.h"

// Testing EMP_CHOOSE_MEMBER_TYPE...
struct A {
  using test_type = double;

  static const int X = 1;
};

struct B {
  static const int X = 2;
};

template <typename T>
struct Wrapper {
  EMP_CHOOSE_MEMBER_TYPE(new_type, test_type, int, T);

  static constexpr new_type VALUE = 5;
};

// Testing EMP_CHOOSE_TYPE_WITH_MEMBER...
struct C {
  static constexpr int class_id = 3;
  bool use_this;
};
struct D {
  static constexpr int class_id = 4;
};
struct E {
  static constexpr int class_id = 5;
  bool use_this;
};
struct F {
  static constexpr int class_id = 6;
};
struct G {
  static constexpr int class_id = 7;
};

EMP_SETUP_TYPE_SELECTOR(auto_type, use_this);

int TestFun(int x, int y, int z) {
  return x+y+z;
}

template <typename T> struct has_XY {
  EMP_ADD_TYPE_FROM_MEMBER(type_X, T, X, double);  // has_XY<A>::type_X = int
  EMP_ADD_TYPE_FROM_MEMBER(type_Y, T, Y, double);  // has_XY<A>::type_Y = double since A::Y does not exist.
};

int main()
{
  // Continuing test of EMP_CHOOSE_MEMBER_TYPE...
  Wrapper<A> wrap_A;
  Wrapper<B> wrap_B;
  (void) wrap_A;
  (void) wrap_B;

  std::cout << Wrapper<A>::VALUE/2 << std::endl;  // 2.5
  std::cout << Wrapper<B>::VALUE/2 << std::endl;  // 2

  // Continuing test of EMP_SETUP_TYPE_SELECTOR...

  std::cout << auto_type<int, C, D, E>::class_id << std::endl;

  std::cout << TestFun(1,2,3) << std::endl;
  std::cout << emp::internal::SubsetCall_impl<int,int,int,int>::Call<>(TestFun, 4,5,6,7.5,8.5) << std::endl;

  std::function<int(int,int,int)> tfun(TestFun);
  std::cout << emp::SubsetCall(tfun, 4,5,6,7.5,8.5) << std::endl;

  has_XY<A>::type_X test_val = (has_XY<A>::type_X) 2.5;
  std::cout << "has_XY<A>::type_X = " << test_val << std::endl;
  has_XY<A>::type_Y test_val2 = (has_XY<A>::type_Y) 2.5;
  std::cout << "has_XY<A>::type_Y = " << test_val2 << std::endl;
}
