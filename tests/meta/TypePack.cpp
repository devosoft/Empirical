/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file TypePack.cpp
 */

#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/TypePack.hpp"

struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

template <typename A, typename B>
struct MetaTestClass { A a; B b; };

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }

TEST_CASE("Test TypePack", "[meta]")
{
  using test_t = emp::TypePack<int, std::string, float, bool, double>;
  // REQUIRE(emp::GetTypeID<test_t>().GetName() == "emp::TypePack<int32_t,std::string,float,bool,double>");
  REQUIRE(test_t::Has<int>() == true);
  REQUIRE(test_t::Has<std::string>() == true);
  REQUIRE(test_t::Has<double>() == true);
  REQUIRE(test_t::Has<char>() == false);
  REQUIRE(test_t::GetSize() == 5);
  REQUIRE(test_t::GetID<float>() == 2);
  REQUIRE(test_t::add<long long>::GetSize() == 6);
  REQUIRE(test_t::pop::GetID<float>() == 1);
  REQUIRE(test_t::IsUnique() == true);

  using test2_t = emp::TypePackFill<int, 4>;
  using test3_t = emp::TypePack<uint64_t>;

  REQUIRE(test2_t::GetSize() == 4);
  REQUIRE(test3_t::GetSize() == 1);

  // Make sure we can use a TypePack to properly set a function type.
  using fun_t = test2_t::to_function_t<int>;
  std::function< fun_t > fun(Sum4);
  REQUIRE(fun(1,2,3,4) == 10);

  using test4_t = test2_t::shrink<2>;
  REQUIRE(test4_t::GetSize() == 2);
  REQUIRE(test_t::merge<test2_t>::GetSize() == 9);

  // IF applied correctly, v will be a vector of uint64_t.
  test3_t::apply<emp::vector> v;
  v.push_back(1);
  v.push_back(2);
  REQUIRE(v.size() == 2);

  // If reverse works correctly, str will be a string.
  test_t::reverse::get<3> str("It worked!");
  REQUIRE(str == "It worked!");

  // Let's try filtering!
  using test_filtered = test_t::filter<std::is_integral>;
  REQUIRE(test_filtered::GetSize() == 2);

  using test_filtered_out = test_t::filter_out<std::is_integral>;
  REQUIRE(test_filtered_out::GetSize() == 3);

  using test_remove = test_t::remove_t<std::string>;
  REQUIRE(test_remove::GetSize() == 4);

  using test_A = emp::TypePack<HasA, std::string, bool, HasA2, HasA, int>;
  // REQUIRE(emp::GetTypeID<test_A>().GetName() == "emp::TypePack<HasA,std::string,bool,HasA2,HasA,int32_t>");

  using test_exist = test_A::filter<MemberA>;
  REQUIRE(test_exist::GetSize() == 3);

  using test_print = test_exist::set<1,int>;
  REQUIRE(test_print::Count<int>() == 1);

  // using wrap_v_t = test_t::wrap<emp::vector>;
  // REQUIRE(emp::GetTypeID<wrap_v_t>().GetName() == "emp::TypePack<emp::vector<int32_t>,emp::vector<std::string>,emp::vector<float>,emp::vector<bool>,emp::vector<double>>");

  // using wrap_A_t = test_A::wrap<MemberA>;
  // REQUIRE(emp::GetTypeID<wrap_A_t>().GetName() == "emp::TypePack<int32_t,char,int32_t>");

  // using shuffle_t = test_t::select<2,3,4,1,3,3,3,0>;
  // REQUIRE(emp::GetTypeID<shuffle_t>().GetName() == "emp::TypePack<float,bool,double,std::string,bool,bool,bool,int32_t>");


  using dup_test_t = emp::TypePack<int, int, double, int, double, std::string, bool, int, char, int>;
  REQUIRE(dup_test_t::GetSize() == 10);
  REQUIRE(dup_test_t::remove_t<int>::GetSize() == 5);
  REQUIRE(dup_test_t::remove_t<double>::GetSize() == 8);
  REQUIRE(dup_test_t::make_unique::GetSize() == 5);
  REQUIRE(dup_test_t::Count<int>() == 5);
  REQUIRE(dup_test_t::CountUnique() == 5);
  REQUIRE(dup_test_t::IsUnique() == false);

  using link1_t = emp::TypePack<bool, char, int>;
  using link2_t = emp::TypePack<double, int, size_t>;
  using merge_t = link1_t::merge<link2_t>;
  using union_t = link1_t::find_union<link2_t>;
  using intersect_t = link1_t::find_intersect<link2_t>;
  REQUIRE(link1_t::GetSize() == 3);
  REQUIRE(link2_t::GetSize() == 3);
  REQUIRE(merge_t::GetSize() == 6);
  REQUIRE(union_t::GetSize() == 5);
  REQUIRE(intersect_t::GetSize() == 1);

  // Test some manipulators on an empty typepack.
  using empty_t = emp::TypePack<>;
  REQUIRE(empty_t::GetSize() == 0);
  REQUIRE(empty_t::Count<int>() == 0);
  REQUIRE(empty_t::IsUnique() == true);
  REQUIRE(empty_t::IsEmpty() == true);

  // Make sure the type trait works...
  REQUIRE(emp::is_TypePack<test_t>() == true);
  REQUIRE(emp::is_TypePack<union_t>() == true);
  REQUIRE(emp::is_TypePack<int>() == false);
  REQUIRE(emp::is_TypePack<std::vector<int>>() == false);
}
