//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//  This file is part of Empirical, https://github.com/devosoft/Empirical
//
//
//  Some examples of code using reflection techniques.

#include <iostream>

#include "meta/reflection.h"

// Testing:
//   EMP_CREATE_METHOD_FALLBACK
//   EMP_CREATE_OPTIONAL_METHOD
//   EMP_CHOOSE_MEMBER_TYPE
//   EMP_CHOOSE_TYPE_WITH_MEMBER
//   EMP_SETUP_TYPE_SELECTOR

struct A {
  using test_type = double;

  static const int X = 1;

  int Test1(int x, int y) { return x + y; }
  void Print(int x, int y) { std::cout << "A:" << Test1(x,y) << std::endl; }

  static std::string GetName() { return "A"; }
};

struct B {
  static const int X = 2;

  int Test1(int x, int y) { return x + 2*y; }
  void Print(int x, int y) { std::cout << "B:" << Test1(x,y) << std::endl; }

  static std::string GetName() { return "B"; }
};

template <typename T>
struct Wrapper {
  EMP_CHOOSE_MEMBER_TYPE(new_type, test_type, int, T)

  static constexpr new_type VALUE = 5;
};

struct C {
  static constexpr int class_id = 3;
  bool use_this;
  int Test1(int x, int y) { return x + 3*y; }
  void Print(int x, int y) { std::cout << "C:" << Test1(x,y) << std::endl; }

  static std::string GetName() { return "C"; }
};

struct D {
  static constexpr int class_id = 4;

  static std::string GetName() { return "D"; }
};

struct E {
  static constexpr int class_id = 5;
  bool use_this = true;

  static std::string GetName() { return "E"; }
};

struct F {
  static constexpr int class_id = 6;

  static std::string GetName() { return "F"; }
};

struct G {
  static constexpr int class_id = 7;

  static std::string GetName() { return "G"; }
};

template <typename T>
int Test1_Fallback(T&, int x, int y) { return x * y; }

EMP_CREATE_METHOD_FALLBACK(Test1, Test1, Test1_Fallback);
EMP_CREATE_OPTIONAL_METHOD(Print, Print);
EMP_CREATE_METHOD_FALLBACK_VAL(Test2, Test1, 888);

EMP_SETUP_TYPE_SELECTOR(auto_type, use_this)

int Add3(int x, int y, int z) {
  return x+y+z;
}

template <typename T> struct has_XY {
  EMP_ADD_TYPE_FROM_MEMBER(type_X, T, X, double);  // has_XY<A>::type_X = int
  EMP_ADD_TYPE_FROM_MEMBER(type_Y, T, Y, double);  // has_XY<A>::type_Y = double since A::Y does not exist.
};

int main()
{
  A a;
  B b;
  C c;
  D d;
  E e;
  F f;
  G g;

  int x = 10;
  int y = 10;
  std::cout << "Test1(a, " << x << ", " << y << ") = " << Test1(a, x, y) << std::endl;
  std::cout << "Test1(b, " << x << ", " << y << ") = " << Test1(b, x, y) << std::endl;
  std::cout << "Test1(c, " << x << ", " << y << ") = " << Test1(c, x, y) << std::endl;
  std::cout << "Test1(d, " << x << ", " << y << ") = " << Test1(d, x, y) << std::endl;
  std::cout << "Test1(e, " << x << ", " << y << ") = " << Test1(e, x, y) << std::endl;
  std::cout << "Test1(f, " << x << ", " << y << ") = " << Test1(f, x, y) << std::endl;
  std::cout << "Test1(g, " << x << ", " << y << ") = " << Test1(g, x, y) << std::endl;

  Print(a,x,y);
  Print(b,x,y);
  Print(c,x,y);
  Print(d,x,y);
  Print(e,x,y);
  Print(f,x,y);
  Print(g,x,y);

  std::cout << "Test2(a, " << x << ", " << y << ") = " << Test2(a, x, y) << std::endl;
  std::cout << "Test2(b, " << x << ", " << y << ") = " << Test2(b, x, y) << std::endl;
  std::cout << "Test2(c, " << x << ", " << y << ") = " << Test2(c, x, y) << std::endl;
  std::cout << "Test2(d, " << x << ", " << y << ") = " << Test2(d, x, y) << std::endl;
  std::cout << "Test2(e, " << x << ", " << y << ") = " << Test2(e, x, y) << std::endl;
  std::cout << "Test2(f, " << x << ", " << y << ") = " << Test2(f, x, y) << std::endl;
  std::cout << "Test2(g, " << x << ", " << y << ") = " << Test2(g, x, y) << std::endl;


  // Continuing test of EMP_CHOOSE_MEMBER_TYPE...
  Wrapper<A> wrap_A;
  Wrapper<B> wrap_B;
  (void) wrap_A;
  (void) wrap_B;

  std::cout << Wrapper<A>::VALUE/2 << std::endl;  // 2.5
  std::cout << Wrapper<B>::VALUE/2 << std::endl;  // 2

  // Continuing test of EMP_SETUP_TYPE_SELECTOR...

  // CE
  std::cout << "Selected (C): " << auto_type<int, C, D, E>::GetName() << std::endl;
  std::cout << "Selected (E): " << auto_type<int, D, E>::GetName() << std::endl;
  std::cout << "Selected (C): " << auto_type<C, D, E>::GetName() << std::endl;

  std::cout << "Add3(1,2,3) = " << Add3(1,2,3) << std::endl;

  std::function<int(int,int,int)> add3(Add3);
  std::cout << "emp::SubsetCall(add3, 4,5,6,7.5,8.5) = "
	    << emp::SubsetCall(add3, 4,5,6,7.5,8.5) << std::endl;

  has_XY<A>::type_X test_val = (has_XY<A>::type_X) 2.5;
  std::cout << "has_XY<A>::type_X = " << test_val << std::endl;
  has_XY<A>::type_Y test_val2 = (has_XY<A>::type_Y) 2.5;
  std::cout << "has_XY<A>::type_Y = " << test_val2 << std::endl;
}
