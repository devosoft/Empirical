/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file ShortString.cpp
 */

#include "../third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/ShortString.hpp"

TEST_CASE("ShortString comparisons", "[tools]")
{
  emp::ShortString str1 = "ABC";
  CHECK( str1.size() == 3 );
  CHECK( str1[0] == 'A' );
  CHECK( str1[1] == 'B' );
  CHECK( str1[2] == 'C' );

  CHECK( str1 == "ABC" );

  std::string std_str = "DEFGH";

  CHECK( str1 != std_str );
  CHECK( str1 < std_str );
  CHECK( str1 <= std_str );

  CHECK( std_str != str1.AsString() );
  CHECK( std_str > str1.AsString() );
  CHECK( std_str >= str1.AsString() );

  emp::ShortString str2(std_str);

  CHECK( str1 != str2 );
  CHECK( str1 < str2 );
  CHECK( str2 > str1 );
  CHECK( str1 <= str2 );
  CHECK( str2 >= str1 );

  str1 = str2;

  CHECK( str1 == str2 );
  CHECK( str1 <= str2);
  CHECK( str1 >= str2);
}

TEST_CASE("ShortString manipulations", "[tools]")
{
  emp::ShortString str1 = "ABC";
  CHECK(str1 == "ABC");
  str1[0] = 'D';
  CHECK(str1 == "DBC");
  str1[1] = 'E';
  CHECK(str1 == "DEC");
  str1[2] = 'F';
  CHECK(str1 == "DEF");

  // str1.push_back('G');
}
