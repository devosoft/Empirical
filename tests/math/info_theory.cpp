#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/info_theory.hpp"

#include <sstream>
#include <string>


TEST_CASE("Test info_theory", "[math]")
{
  emp::vector<int> weights = { 100, 100, 200 };
  REQUIRE( emp::Entropy(weights) == 1.5 );

  emp::vector<double> dweights = { 10.5, 10.5, 10.5, 10.5, 21.0, 21.0 };
  REQUIRE( emp::Entropy(dweights) == 2.5 );

  REQUIRE( emp::Entropy2(0.5) == 1.0 );
}
