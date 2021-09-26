/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file reflection.cpp
 */

#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/tuple_utils.hpp"
#include "emp/meta/reflection.hpp"


struct HasA { static int A; static std::string TypeID() { return "HasA"; } };
struct HasA2 { static char A; };
template <typename T> using MemberA = decltype(T::A);

TEST_CASE("Test reflection", "[meta]")
{
  REQUIRE((emp::test_type<MemberA,int>()) == false);
  REQUIRE((emp::test_type<MemberA,HasA>()) == true);
  REQUIRE((emp::test_type<MemberA,HasA2>()) == true);
  REQUIRE((emp::test_type<std::is_integral,int>()) == true);
  REQUIRE((emp::test_type<std::is_integral,HasA>()) == false);
  REQUIRE((emp::test_type<std::is_integral, HasA2>()) == false);

  // @CAO Need to build more reflection tests, once reflection is restructured.
}
