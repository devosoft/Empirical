#include <vector>
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "geometry/Body2D.h"
#include "geometry/Circle2D.h"
#include "geometry/Point2D.h"


TEST_CASE("Test Body2d", "[geometry]")
{
  emp::Circle body_outline(10.0);
  emp::CircleBody2D body1(body_outline);


  // Test to make sure shifts and pressure are being calculated correctly.

  emp::Point shift1(5.0, 6.0);
  emp::Point shift2(-2.0, -2.0);

  body1.AddShift(shift1);

  REQUIRE(body1.GetShift() == shift1);

  body1.AddShift(shift2);

  REQUIRE(body1.GetShift() == shift1 + shift2);
  //BAD TEST: REQUIRE(body1.CalcPressure() == 32);

  // Start a round of replication for tests

  emp::Ptr<emp::CircleBody2D> body2 = body1.BuildOffspring({3.0, -4.0});
  
  // Make sure original organism is linked to offspring.
  REQUIRE(body1.IsLinked(*body2));
  REQUIRE(body2->IsLinked(body1));
  REQUIRE(body1.GetLinkDist(*body2) == 5.0);
  REQUIRE(body2->GetLinkDist(body1) == 5.0);
  REQUIRE(body1.GetTargetLinkDist(*body2) == 20.0);
  REQUIRE(body2->GetTargetLinkDist(body1) == 20.0);

  REQUIRE(body1.GetTargetLinkDist(*body2) == 20);
  REQUIRE(body2->GetTargetLinkDist(body1) == 20);
  body2.Delete();
}


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
