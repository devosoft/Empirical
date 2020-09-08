//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#undef NDEBUG
#define TDEBUG 1

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "base/assert.h"
#include "base/optional.h"

#include <utility>
#include <sstream>
#include <iostream>

TEST_CASE("Test optional bad access", "[base]") {

  emp::optional<std::string> opt;

  REQUIRE( !opt.has_value() );

  REQUIRE(!emp::assert_last_fail);
  opt->size();
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *opt;
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *std::move(opt);
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  {
  auto& optref = opt;

  REQUIRE(!emp::assert_last_fail);
  opt->size();
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *optref;
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *std::move(optref);
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();
  }

  {
  const auto& optref = opt;

  REQUIRE(!emp::assert_last_fail);
  opt->size();
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *optref;
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();

  REQUIRE(!emp::assert_last_fail);
  *std::move(optref);
  REQUIRE(emp::assert_last_fail);
  emp::assert_clear();
  }

}

TEST_CASE("Test optional good access", "[base]") {

  REQUIRE(!emp::assert_last_fail);

  emp::optional<std::string> opt{ "howdy" };

  REQUIRE( opt.has_value() );

  REQUIRE( opt->size() == 5);

  REQUIRE( *opt == "howdy" );
  REQUIRE( *std::move(opt) == "howdy" );

  REQUIRE( opt.value() == "howdy" );
  REQUIRE( std::move(opt).value() == "howdy" );

  {
  auto& optref = opt;
  REQUIRE( *optref == "howdy" );
  REQUIRE( *std::move(optref) == "howdy" );

  REQUIRE( optref.value() == "howdy" );
  REQUIRE( std::move(optref).value() == "howdy" );
  }

  {
  const auto& optref = opt;
  REQUIRE( *optref == "howdy" );
  REQUIRE( *std::move(optref) == "howdy" );

  REQUIRE( optref.value() == "howdy" );
  REQUIRE( std::move(optref).value() == "howdy" );
  }

  REQUIRE(!emp::assert_last_fail);

}
