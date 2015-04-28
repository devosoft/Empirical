#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../geometry/Circle2D.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Point<double> test_point(4.0,3.0);

  emp::Circle<double> circle0(7.0);
  emp::Circle<double> circle_small(test_point, 1.5);
  emp::Circle<double> circle_big(test_point, 5.0);

  emp_assert(circle0.Contains(circle_small) == true);
  emp_assert(circle0.Contains(circle_big) == false);
  emp_assert(circle0.HasOverlap(circle_big) == false);
  
  emp::Point<double> test_point2(6.0,5.0);
  emp_assert(circle_small.Contains(test_point2) == false);
  emp_assert(circle_big.Contains(test_point2) == true);
}
