#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/Cache.hpp"

#include <sstream>
#include <string>

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