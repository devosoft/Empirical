/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file timing.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/timing.hpp"

TEST_CASE("Test Functions", "[tools]")
{
  REQUIRE(emp::TimeFun([] () { return 2*2; }) < 2.0);

}
