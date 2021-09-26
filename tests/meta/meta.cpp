/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file meta.cpp
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


TEST_CASE("Test meta-programming helpers (meta.h)", "[meta]")
{
  // TEST FOR VARIADIC HELPER FUNCTIONS:

  REQUIRE((emp::get_type_index<char, char, bool, int, double>()) == 0);
  REQUIRE((emp::get_type_index<int, char, bool, int, double>()) == 2);
  REQUIRE((emp::get_type_index<double, char, bool, int, double>()) == 3);
  REQUIRE((emp::get_type_index<std::string, char, bool, int, double>()) < 0);

  REQUIRE((emp::has_unique_first_type<int, bool, std::string, bool, char>()) == true);
  REQUIRE((emp::has_unique_first_type<bool, int, std::string, bool, char>()) == false);
  REQUIRE((emp::has_unique_types<bool, int, std::string, emp::vector<bool>, char>()) == true);
  REQUIRE((emp::has_unique_types<int, bool, std::string, bool, char>()) == false);


  using meta1_t = MetaTestClass<int, double>;
  using meta2_t = emp::AdaptTemplate<meta1_t, char, bool>;
  using meta3_t = emp::AdaptTemplate_Arg1<meta1_t, std::string>;

  meta1_t meta1;
  meta2_t meta2;
  meta3_t meta3;

  meta1.a = (decltype(meta1.a)) 65.5;
  meta1.b = (decltype(meta1.b)) 65.5;
  meta2.a = (decltype(meta2.a)) 65.5;
  meta2.b = (decltype(meta2.b)) 65.5;
  meta3.a = (decltype(meta3.a)) "65.5";
  meta3.b = (decltype(meta3.b)) 65.5;

  REQUIRE( meta1.a == 65 );
  REQUIRE( meta1.b == 65.5 );
  REQUIRE( meta2.a == 'A' );
  REQUIRE( meta2.b == true );
  REQUIRE( meta3.a == "65.5" );
  REQUIRE( meta3.b == 65.5 );

  // Combine hash should always return the original values if only one combined.
  REQUIRE( emp::CombineHash(1) == 1 );
  REQUIRE( emp::CombineHash(2) == std::hash<int>()(2) );
  REQUIRE( emp::CombineHash(3) == std::hash<int>()(3) );
  REQUIRE( emp::CombineHash(4) == std::hash<int>()(4) );
  REQUIRE( emp::CombineHash(2,3) == 0x9e377a3e );
  REQUIRE( emp::CombineHash(3,2) == 0x9e377a78);
  REQUIRE( emp::CombineHash(1,2) == 0x9e3779fa);
  REQUIRE( emp::CombineHash(3,4) == 0x9e377a7e);
  REQUIRE( emp::CombineHash(2,3,4) == 0x13c6ef4fc );

}

TEST_CASE("Test GetSize", "[meta]") {
  int some_ints[] = {1, 2, 4, 8};
  REQUIRE(emp::GetSize(some_ints) == 4);
}
