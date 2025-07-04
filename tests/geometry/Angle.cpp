/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/geometry/Angle.hpp"

TEST_CASE("Test Angle2D", "[geometry]")
{
  emp::Angle angle;
  CHECK(angle == emp::Angle::UP());
  angle.RotateRight();
  CHECK(angle == emp::Angle::RIGHT());
  angle.RotateUTurn();
  CHECK(angle == emp::Angle::LEFT());
  angle.RotateLeft();
  CHECK(angle == emp::Angle::DOWN());

  CHECK(angle.AsPortion() == 0.5);
  CHECK(angle.AsRadians() == emp::PI);
  CHECK(angle.AsDegrees() == 180);

  angle = emp::Angle::RIGHT() * 3;
  CHECK(angle == emp::Angle::LEFT());
}
