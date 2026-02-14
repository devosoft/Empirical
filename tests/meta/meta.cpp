/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021
*/
/**
 *  @file
 */

#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/meta.hpp"

struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

template <typename A, typename B>
struct MetaTestClass { A a; B b; };

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }


TEST_CASE("Test pack-indexing helpers (meta.h)", "[meta]")
{
  CHECK( std::same_as<emp::first_type <int, std::string, double, char, bool, float, long long>,  int> );
  CHECK( std::same_as<emp::second_type<int, std::string, double, char, bool, float, long long>,  std::string> );
  CHECK( std::same_as<emp::third_type <int, std::string, double, char, bool, float, long long>,  double> );
  CHECK( std::same_as<emp::fourth_type<int, std::string, double, char, bool, float, long long>,  char> );

  CHECK( std::same_as<emp::pack_id<1, int, std::string, double, char, bool, float, long long>,  std::string> );
  CHECK( std::same_as<emp::pack_id<3, int, std::string, double, char, bool, float, long long>,  char> );
  CHECK( std::same_as<emp::pack_id<5, int, std::string, double, char, bool, float, long long>,  float> );

  CHECK( std::same_as<emp::last_type<int, std::string, double, char, bool, float, long long>,  long long> );
}


TEST_CASE("Pack membership / counting / uniqueness (meta.h)", "[meta]")
{
  CHECK( emp::has_type<int,        std::string, double, char, bool, float, long long>() == false);
  CHECK( emp::has_type<char,       std::string, double, char, bool, float, long long>() == true);
  CHECK( emp::has_type<long long,  std::string, double, char, bool, float, long long>() == true);

  CHECK( emp::count_type<int,  int, int, int, int>() == 4);
  CHECK( emp::count_type<int,  char, char, char, char>() == 0);
  CHECK( emp::count_type<int,  char, int, bool, float>() == 1);

  CHECK((emp::has_unique_types<bool, int, std::string, emp::vector<bool>, char>()) == true);
  CHECK((emp::has_unique_types<int, bool, std::string, bool, char>()) == false);
}

TEST_CASE("Pack indexing (meta.h)", "[meta]")
{
  CHECK((emp::get_type_index<char,   char, bool, int, double>()) == 0);
  CHECK((emp::get_type_index<int,    char, bool, int, double>()) == 2);
  CHECK((emp::get_type_index<double, char, bool, int, double>()) == 3);
  CHECK((emp::get_type_index<std::string, char, bool, int, double>()) < 0);
}


TEST_CASE("Test GetSize", "[meta]") {
  int some_ints[] = {1, 2, 4, 8};
  CHECK(emp::GetSize(some_ints) == 4);
}
