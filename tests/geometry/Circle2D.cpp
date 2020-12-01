#include <vector>
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"
#include <stdlib.h>

#include "emp/geometry/Circle2D.hpp"

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

  emp::Circle circle_xyr(7.0, 10.0, 3.0);
  REQUIRE(circle_xyr.GetCenterX() == 7.0);
  REQUIRE(circle_xyr.GetCenterY() == 10.0);

  circle_xyr.SetCenter(4.0,9.0);
  REQUIRE(circle_xyr.GetCenterX() == 4.0);
  REQUIRE(circle_xyr.GetCenterY() == 9.0);

  circle_xyr.SetCenterY(11.0);
  REQUIRE(circle_xyr.GetCenterY() == 11.0);

  circle_xyr.SetCenterX(2.0);
  REQUIRE(circle_xyr.GetCenterX() == 2.0);

  circle_xyr.SetRadius(7.0);
  REQUIRE(circle_xyr.GetRadius()== 7.0);

  circle_xyr.Set(1.0,9.0,2.5);
  REQUIRE(circle_xyr.GetCenterX() == 1.0);
  REQUIRE(circle_xyr.GetCenterY() == 9.0);
  REQUIRE(circle_xyr.GetRadius() == 2.5);

  REQUIRE(circle_xyr.Contains(1.5,9.5) == true);



}
