/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Cache.cpp
 */

#include <sstream>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/Cache.hpp"

TEST_CASE("Test Cache", "[datastructs]")
{
  emp::Cache<char,int> cch;
  std::function<int(char)> cch_fx = [](char ch){ return (int)ch; };
  REQUIRE((cch.size() == 0));
  REQUIRE((cch.Get('a',cch_fx) == 97));
  REQUIRE((cch.size() == 1));
  REQUIRE(cch.Has('a'));
  REQUIRE((cch.Get('b',cch_fx) == 98));
  REQUIRE((cch.size() == 2));
  cch.Clear();
  REQUIRE((cch.size() == 0));
  const char c = 'c';
  REQUIRE((cch.GetRef(c,cch_fx) == 99));
  REQUIRE(!cch.Has('a'));
  REQUIRE(!cch.Has('b'));
  REQUIRE(cch.Has(c));
}
