#include <vector>
#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/geometry/Body2D.hpp"


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


  //Simple test creating and getting birth time

  emp::CircleBody2D body3(body_outline);
  body3.SetBirthTime(1.1);
  REQUIRE(body3.GetBirthTime() == 1.1);


}
