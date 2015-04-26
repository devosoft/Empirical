#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../geometry/Point2D.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // Test constructors...
  emp::Point<double> point_base;              // (0,0)
  emp::Point<double> point_set(1.5, 2.0);
  emp::Point<double> point_copy(point_set);
  emp::Point<double> point_scale(point_set, 5.0);

  emp_assert(point_base.Magnitude() == 0.0);
  emp_assert(point_set.Magnitude() == 2.5);
  emp_assert(point_copy.Magnitude() == 2.5);
  emp_assert(point_scale.Magnitude() == 5.0);

  if (verbose) {
    std::cout << "Constructor tests passed." << std::endl;
  }
}
