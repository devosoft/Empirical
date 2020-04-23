#include <vector>
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "geometry/Circle2D.h"

TEST_CASE("Test Circle2D", "[geometry]")
{

  emp::Point test_point(4.0,3.0);

  emp::Circle circle0(7.0);
  emp::Circle circle_small(test_point, 1.5);
  emp::Circle circle_big(test_point, 5.0);

  REQUIRE(circle0.Contains(circle_small) == true);
  REQUIRE(circle0.Contains(circle_big) == false);
  //BAD TEST: REQUIRE(circle0.HasOverlap(circle_big) == false);

  emp::Point test_point2(6.0,5.0);
  REQUIRE(circle_small.Contains(test_point2) == false);
  REQUIRE(circle_big.Contains(test_point2) == true);
}
