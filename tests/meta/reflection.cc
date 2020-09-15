#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "base/array.h"
#include "base/vector.h"
#include "base/Ptr.h"
#include "tools/tuple_utils.h"

#include "meta/reflection.h"


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
