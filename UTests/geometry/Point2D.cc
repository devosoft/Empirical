#include <iostream>
#include <vector>

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
  emp::Point<double> point_list({3.0, 4.0});

  emp_assert(point_base.Magnitude() == 0.0);
  emp_assert(point_set.Magnitude() == 2.5);
  emp_assert(point_copy.Magnitude() == 2.5);
  emp_assert(point_scale.Magnitude() == 5.0);

  if (verbose) {
    std::cout << "Constructor tests passed." << std::endl;
  }

  // Test comparisons
  emp_assert(point_set == point_copy);
  emp_assert(point_set != point_scale);
  emp_assert(point_scale == point_list);

  if (verbose) {
    std::cout << "Comparison tests passed." << std::endl;
  }

  // Test setting points
  std::vector<emp::Point<double> > points(10);
  points[0].SetX(13);
  points[1].SetY(13);
  points[2].Set(5, 12);

  emp_assert(points[0].Magnitude() == 13);
  emp_assert(points[1].Magnitude() == 13);
  emp_assert(points[2].Magnitude() == 13);

  if (verbose) {
    std::cout << "Point Setting tests passed." << std::endl;
  }


  // Explicitly test magnitude-related member functions
  
}
