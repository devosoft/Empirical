#include <vector>
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/geometry/Point2D.hpp"


TEST_CASE("Test Point2D", "[geometry]")
{
  // Test constructors...
  emp::Point point_base;              // (0,0)
  emp::Point point_set(1.5, 2.0);
  emp::Point point_copy(point_set);
  emp::Point point_scale(point_set, 5.0);
  emp::Point point_list({3.0, 4.0});

  REQUIRE(point_base.Magnitude() == 0.0);
  REQUIRE(point_set.Magnitude() == 2.5);
  REQUIRE(point_copy.Magnitude() == 2.5);
  REQUIRE(point_scale.Magnitude() == 5.0);

  // Test comparisons
  REQUIRE(point_set == point_copy);
  REQUIRE(point_set != point_scale);
  REQUIRE(point_scale == point_list);

  // Test setting points
  std::vector<emp::Point > points(10);
  points[0].SetX(13);
  points[1].SetY(13);
  points[2].Set(5, 12);

  REQUIRE(points[0].Magnitude() == 13);
  REQUIRE(points[1].Magnitude() == 13);
  REQUIRE(points[2].Magnitude() == 13);

  // Test if at origin...

  REQUIRE(points[0].AtOrigin() == false);
  REQUIRE(points[1].AtOrigin() == false);
  REQUIRE(points[2].AtOrigin() == false);
  REQUIRE(points[3].AtOrigin() == true);

  REQUIRE(points[0].NonZero() == true);
  REQUIRE(points[1].NonZero() == true);
  REQUIRE(points[2].NonZero() == true);
  REQUIRE(points[3].NonZero() == false);

  // Test Midpoint and rotations

  points[4].Set(4,4);
  points[5].Set(5,5);

  REQUIRE(points[4].GetMidpoint(points[5]) == emp::Point(4.5,4.5));
  REQUIRE(points[5].GetMidpoint(points[4]) == emp::Point(4.5,4.5));
  REQUIRE(points[2].GetRot90().Magnitude() == 13);
  REQUIRE(points[2].GetRot180().Magnitude() == 13);
  REQUIRE(points[2].GetRot270().Magnitude() == 13);
}

