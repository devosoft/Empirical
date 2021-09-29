#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/DisjointVariant.hpp"

#include <iostream>
#include <tuple>
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

  // ensure that disjoint variant is zero overhead,
  // that there's only one copy of a big type in there
  static_assert(
    sizeof(emp::DisjointVariant<std::array<int, 99>>)
    < 2 * sizeof(std::array<int, 99>)
  );
  // that overhead scales 1:1 with data
  static_assert(
    sizeof(emp::DisjointVariant<int, char, double>)
      - sizeof(emp::DisjointVariant<char, double>)
    == sizeof(std::tuple<int, char, double>)
      - sizeof(std::tuple<char, double>)
  );
  // that overhead is <= uint64_t
  static_assert(
    sizeof(emp::DisjointVariant<char>) <= sizeof(uint64_t) + 1
  );

}
