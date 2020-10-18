//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/assert.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"

#include <utility>
#include <sstream>
#include <iostream>

TEST_CASE("Test optional bad access", "[base]") {

  emp::optional<std::string> opt;

  REQUIRE( !opt.has_value() );

  #ifndef NDEBUG // these should only throw in debug mode, so only test then

  REQUIRE_THROWS_AS( opt->size(), std::bad_optional_access );

  REQUIRE_THROWS_AS( *opt, std::bad_optional_access );

  REQUIRE_THROWS_AS( *std::move(opt), std::bad_optional_access );

  {
  auto& optref = opt;

  REQUIRE_THROWS_AS( optref->size(), std::bad_optional_access );

  REQUIRE_THROWS_AS( *optref, std::bad_optional_access );

  REQUIRE_THROWS_AS( *std::move(optref), std::bad_optional_access );
  }

  {
  const auto& optref = opt;

  REQUIRE_THROWS_AS( optref->size(), std::bad_optional_access );

  REQUIRE_THROWS_AS( *optref, std::bad_optional_access );

  REQUIRE_THROWS_AS( *std::move(optref), std::bad_optional_access );
  }

  #endif

}

TEST_CASE("Test optional good access", "[base]") {

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

}

TEST_CASE("Test make_optional", "[base]") {

  {
  auto res = emp::make_optional(7);
  REQUIRE( *res == 7 );
  }

  {
  auto res = emp::make_optional<std::string>(3ul, 'a');
  REQUIRE( *res == "aaa" );
  }

  {
  auto res = emp::make_optional<emp::vector<char>>({'a', 'b', 'c'});
  REQUIRE( *res == emp::vector<char>{'a', 'b', 'c'} );
  }

}
