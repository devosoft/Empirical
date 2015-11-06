// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../geometry/Body2D.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Circle<double> body_outline(10.0);
  emp::CircleBody2D<int, double> body1(body_outline);


  // Test to make sure shifts and pressure are being calculated correctly.

  emp::Point<double> shift1(5.0, 6.0);
  emp::Point<double> shift2(-2.0, -2.0);

  body1.AddShift(shift1);

  emp_assert(body1.GetShift() == shift1);

  body1.AddShift(shift2);

  emp_assert(body1.GetShift() == shift1 + shift2);
  emp_assert(body1.CalcPressure() == 32);

  if (verbose) {
    std::cout << "Shifting & Pressure tests passed." << std::endl;
  }

  // Start a round of replication for tests

  emp::CircleBody2D<int, double> & body2 = *(body1.BuildOffspring({3.0, -4.0}));

  // Make sure original organism is linked to offspring.
  emp_assert(body1.IsLinked(body2));
  emp_assert(body2.IsLinked(body1));
  emp_assert(body1.GetLinkDist(body2) == 5.0);
  emp_assert(body2.GetLinkDist(body1) == 5.0);
  emp_assert(body1.GetTargetLinkDist(body2) == 20.0);
  emp_assert(body2.GetTargetLinkDist(body1) == 20.0);

  std::cout << body1.GetTargetLinkDist(body2) << std::endl;
  std::cout << body2.GetTargetLinkDist(body1) << std::endl;

  (void) body2;
  
}
