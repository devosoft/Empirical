//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using TypePack

#include <iostream>
#include <string>
#include <vector>

#include "meta/TypeID.h"
#include "meta/TypePack.h"

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }

struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

int main()
{
  using test_t = emp::TypePack<int, std::string, float, bool, double>;

  std::cout << "test_t = " << emp::GetTypeID<test_t>().GetName() << std::endl;

  std::cout << "Num types = " << test_t::GetSize() << std::endl;
  std::cout << "float pos = " << test_t::GetID<float>() << std::endl;

  std::cout << "Num types (with add) = " << test_t::add<long long>::GetSize() << std::endl;
  std::cout << "float pos (with pop) = " << test_t::pop::GetID<float>() << std::endl;

  using test2_t = emp::TypePackFill<int, 4>;
  using test3_t = emp::TypePack<uint64_t>;

  std::cout << std::endl;
  std::cout << "Num test_t types:  " << test_t::GetSize() << std::endl;
  std::cout << "Num test2_t types: " << test2_t::GetSize() << std::endl;
  std::cout << "Num test3_t types: " << test3_t::GetSize() << std::endl;

  // Make sure we can use a TypePack to properly set a function type.
  using fun_t = test2_t::to_function_t<int>;
  std::function< fun_t > fun(Sum4);
  std::cout << "Sum4(1,2,3,4) = " << fun(1,2,3,4) << std::endl;

  using test4_t = test2_t::shrink<2>;
  std::cout << "After shrink<2>, size of type = " << test4_t::GetSize() << std::endl;
  std::cout << "After merge of t1 and t2, size = "
	    << test_t::merge<test2_t>::GetSize() << std::endl;

  // IF applied correctly, v will be a vector of uint64_t.
  test3_t::apply<std::vector> v;
  v.push_back(1);

  // If reverse works correctly, str will be a string.
  test_t::reverse::get<3> str("It worked!");
  std::cout << "Can we reverse a TypePack? -> " << str << std::endl;

  // Let's try filtering!
  using test_filtered = test_t::filter<std::is_integral>;
  std::cout << "Number of integral types in test_t = " << test_filtered::GetSize() << std::endl;


  using test_A = emp::TypePack<HasA, std::string, bool, HasA2, HasA, int>;
  std::cout << "test_A = " << emp::GetTypeID<test_A>().GetName() << std::endl;

  using test_exist = test_A::filter<MemberA>;
  std::cout << "Number that have a member A = " << test_exist::GetSize() << std::endl;

  using test_print = test_exist::set<1,int>;
  test_print x;
  (void) x;

  std::cout << "\nMore filter tests...\n";
  std::cout << "emp::test_type<MemberA, int>() = " << emp::test_type<MemberA, int>() << std::endl;
  std::cout << "emp::test_type<MemberA, HasA>() = " << emp::test_type<MemberA, HasA>() << std::endl;
  std::cout << "emp::test_type<MemberA, HasA2>(); = " << emp::test_type<MemberA, HasA2>() << std::endl;
  std::cout << "emp::test_type<std::is_integral, int>() = " << emp::test_type<std::is_integral, int>() << std::endl;
  std::cout << "emp::test_type<std::is_integral, HasA>() = " << emp::test_type<std::is_integral, HasA>() << std::endl;
  std::cout << "emp::test_type<std::is_integral, HasA2>() = " << emp::test_type<std::is_integral, HasA2>() << std::endl;

  using wrap_v_t = test_t::wrap<std::vector>;
  std::cout << "wrap_v_t = " << emp::GetTypeID<wrap_v_t>().GetName() << std::endl;
  using wrap_A_t = test_A::wrap<MemberA>;
  std::cout << "wrap_A_t = " << emp::GetTypeID<wrap_A_t>().GetName() << std::endl;

  std::cout << std::endl;
  using shuffle_t = test_t::select<2,3,4,1,3,3,3,0>;
  std::cout << "Shuffle with test_t::select<>:" << emp::GetTypeID<shuffle_t>().GetName() << std::endl;
}
