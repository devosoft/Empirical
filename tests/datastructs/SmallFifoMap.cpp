/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file SmallFifoMap.cpp
 */

#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/SmallFifoMap.hpp"

TEST_CASE("SmallFifoMap") {

  emp::SmallFifoMap<size_t, std::string, 2> cache;

  REQUIRE( cache.capacity() == 2 );

  REQUIRE( cache.empty() );
  REQUIRE( cache.size() == 0 );
  REQUIRE( cache.get( 42 ) == nullptr );

  cache.set( 42, "42" );
  REQUIRE( *cache.get( 42 ) == "42" );
  REQUIRE( cache[ 42 ] == "42" );
  REQUIRE( cache.size() == 1 );
  REQUIRE( !cache.empty() );

  cache.set( 101, "101" );
  REQUIRE( *cache.get( 42 ) == "42" );
  REQUIRE( *cache.get( 101 ) == "101" );
  REQUIRE( cache[ 42 ] == "42" );
  REQUIRE( cache[ 101 ] == "101" );
  REQUIRE( cache.size() == 2 );
  REQUIRE( !cache.empty() );

  cache.set( 12, "12" );
  REQUIRE( *cache.get( 12 ) == "12" );
  REQUIRE( *cache.get( 101 ) == "101" );
  REQUIRE( cache[ 12 ] == "12" );
  REQUIRE( cache[ 101 ] == "101" );
  REQUIRE( cache.get( 42 ) == nullptr );
  REQUIRE( cache.size() == 2 );
  REQUIRE( !cache.empty() );

  cache.clear();

  REQUIRE( cache.empty() );
  REQUIRE( cache.size() == 0 );
  REQUIRE( cache.get( 12 ) == nullptr );
  REQUIRE( cache.get( 101 ) == nullptr );
  REQUIRE( cache.get( 42 ) == nullptr );

}
