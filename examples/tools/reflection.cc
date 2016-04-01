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

EMP_CHOOSE_TYPE_WITH_MEMBER(auto_type, use_this, int, C, D, E);


/*
 * template <typename EMP__T, typename... EXTRAS>
struct ABC {
  template <typename T> static EMP__T GetType(typename emp::sfinae_decoy<bool, decltype(T::use_this)>::type);
  template <typename T> static typename ABC<EXTRAS...>::type GetType(...);

  using type = decltype(GetType<EMP__T>(true));
};
template <> struct ABC<void> { using type = void; };
using auto_type = typename ABC<int, bool, F, G, D, E, C, D, E, void>::type;
*/

int main()
{
  // Continuing test of EMP_CHOOSE_MEMBER_TYPE...
  Wrapper<A> wrap_A;
  Wrapper<B> wrap_B;

  std::cout << Wrapper<A>::VALUE/2 << std::endl;  // 2.5
  std::cout << Wrapper<B>::VALUE/2 << std::endl;  // 2

  // Continuing test of EMP_CHOOSE_TYPE_WITH_MEMBER...
  std::cout << auto_type::class_id << std::endl;
}
