#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/DisjointVariant.hpp"

#include <iostream>
#include <type_traits>

TEST_CASE("Test DisjointVariant", "[datastructs]") {

  emp::DisjointVariant<int, double> disjoint_variant{10, 8.2};

  disjoint_variant.Visit( [](auto&& v){ REQUIRE( v == 10 ); } );

  // write a new value into disjoint_variant's int slot
  disjoint_variant.Visit( [](auto&& v){ v = 42; } );
  disjoint_variant.Visit( [](auto&& v){ REQUIRE( v == 42 ); } );

  disjoint_variant.Activate<double>();
  disjoint_variant.Visit( [](auto&& v){ REQUIRE( v == 8.2 ); } );

  disjoint_variant.Activate<int>();
  disjoint_variant.Visit( [](auto&& v){ REQUIRE( v == 42 ); } );

}
