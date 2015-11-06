// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

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

  // Test if at origin...

  emp_assert(points[0].AtOrigin() == false);
  emp_assert(points[1].AtOrigin() == false);
  emp_assert(points[2].AtOrigin() == false);
  emp_assert(points[3].AtOrigin() == true);
  
  emp_assert(points[0].NonZero() == true);
  emp_assert(points[1].NonZero() == true);
  emp_assert(points[2].NonZero() == true);
  emp_assert(points[3].NonZero() == false);

  if (verbose) {
    std::cout << "At origin tests passed." << std::endl;
  }

  // Test Midpoint and rotations

  points[4].Set(4,4);
  points[5].Set(5,5);

  emp_assert(points[4].GetMidpoint(points[5]) == emp::Point<double>(4.5,4.5));
  emp_assert(points[5].GetMidpoint(points[4]) == emp::Point<double>(4.5,4.5));
  emp_assert(points[2].GetRot90().Magnitude() == 13);
  emp_assert(points[2].GetRot180().Magnitude() == 13);
  emp_assert(points[2].GetRot270().Magnitude() == 13);
}
