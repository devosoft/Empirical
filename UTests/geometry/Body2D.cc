#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../geometry/Body2D.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Circle<double> body_outline(5.0);
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

  emp::CircleBody2D<int, double> * body2 = body1.BuildOffspring({0.3, 0.4});
  (void) body2;
  
}
