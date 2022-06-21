/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file ShortString.cpp
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/ShortString.hpp"

TEST_CASE("Test ShortString", "[tools]")
{
  emp::ShortString str1 = "ABC";
  CHECK( str1.size() == 3 );
  CHECK( str1[0] == 'A' );
  CHECK( str1[1] == 'B' );
  CHECK( str1[2] == 'C' );

  CHECK( str1 = "ABC" );

}
