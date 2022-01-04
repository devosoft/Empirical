/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file RankedSelector.cpp
 */

#include <ratio>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/vector.hpp"
#include "emp/matching/selectors_static/RankedSelector.hpp"

using tenth = std::ratio<1, 10>;

TEST_CASE("N = 1, nothing to select on", "[tools]") {

  const auto res = emp::statics::RankedSelector<>::select( {} );

  REQUIRE( res.size() == 0 );

}

TEST_CASE("N = 1, zero results below threshold", "[tools]") {

  emp::vector<float> scores{0.5f, 0.42f, 0.42f, 0.9f, 0.2f};

  const auto res = emp::statics::RankedSelector<tenth>::select( scores );

  REQUIRE( res.size() == 0 );

}

TEST_CASE("N = 1, one result below threshold", "[tools]") {

  emp::vector<float> scores{0.5f, 0.42f, 0.42f, 0.9f, 0.09f};

  const auto res = emp::statics::RankedSelector<tenth>::select( scores );

  REQUIRE( res.size() == 1 );
  REQUIRE( scores[res[0]] == 0.09f );

}

TEST_CASE("N = 1, two results below threshold", "[tools]") {

  emp::vector<float> scores{0.05f, 0.42f, 0.42f, 0.9f, 0.09f};

  const auto res = emp::statics::RankedSelector<tenth>::select( scores );

  REQUIRE( res.size() == 1 );
  REQUIRE( scores[res[0]] == 0.05f );

}

TEST_CASE("N = 2, nothing to select on", "[tools]") {

  const auto res = emp::statics::RankedSelector<tenth, 2>::select( {} );

  REQUIRE( res.size() == 0 );

}

TEST_CASE("N = 2, zero results below threshold", "[tools]") {

  const auto res = emp::statics::RankedSelector<tenth, 2>::select( {0.2f} );

  REQUIRE( res.size() == 0 );

}

TEST_CASE("N = 2, one result below threshold", "[tools]") {

  emp::vector<float> scores{0.5f, 0.42f, 0.042f, 0.9f, 0.2f};

  const auto res = emp::statics::RankedSelector<tenth, 2>::select( scores );

  REQUIRE( res.size() == 1 );
  REQUIRE( scores[res[0]] == 0.042f );

}

TEST_CASE("N = 2, two results below threshold", "[tools]") {

  emp::vector<float> scores{0.05f, 0.42f, 0.042f, 0.9f, 0.2f};

  const auto res = emp::statics::RankedSelector<tenth, 2>::select( scores );

  REQUIRE( res.size() == 2 );
  REQUIRE(( 0.042f == scores[res[0]] || 0.042f == scores[res[1]] ));
  REQUIRE(( 0.05f == scores[res[0]] || 0.05f == scores[res[1]] ));

}

TEST_CASE("N = 2, three results below threshold", "[tools]") {

  emp::vector<float> scores{0.05f, 0.42f, 0.042f, 0.009f, 0.2f};

  const auto res = emp::statics::RankedSelector<tenth, 2>::select( scores );

  REQUIRE( res.size() == 2 );
  REQUIRE(( 0.042f == scores[res[0]] || 0.042f == scores[res[1]] ));
  REQUIRE(( 0.009f == scores[res[0]] || 0.009f == scores[res[1]] ));

}
