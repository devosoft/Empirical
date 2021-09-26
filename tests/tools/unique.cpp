
#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/unique.hpp"

TEST_CASE("Test Functions", "[tools]")
{
    REQUIRE(emp::UniqueVal() == 0);
    REQUIRE(emp::UniqueVal() == 1);
  REQUIRE(emp::UniqueName("string") == "string2");
}
