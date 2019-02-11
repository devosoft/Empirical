//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>

#include "base/assert.h"
#include "base/vector.h"
#include "config/command_line.h"
#include "web/color_map.h"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  const auto & test_map = emp::GetHueMap(30);
  const auto & test_map2 = emp::GetHueMap(60);
  const auto & test_map3 = emp::GetHSLMap(30);
  const auto & test_map4 = emp::GetHSLMap(60);

  for (size_t i = 0; i < test_map.size(); ++i) {
    if (verbose) std::cout << test_map[i] << std::endl;
  }
  for (size_t i = 0; i < test_map2.size(); ++i) {
    if (verbose) std::cout << test_map2[i] << std::endl;
  }
  for (size_t i = 0; i < test_map3.size(); ++i) {
    if (verbose) std::cout << test_map3[i] << std::endl;
  }
  for (size_t i = 0; i < test_map4.size(); ++i) {
    if (verbose) std::cout << test_map4[i] << std::endl;
  }

  emp_assert(emp::ColorRGB(255,255,255) == "#ffffff");
  emp_assert(emp::ColorRGB(0,0,0) == "#000000");
  emp_assert(emp::ColorRGB(0,255,0) == "#00ff00");
  emp_assert(emp::ColorRGB(26,28,147) == "#1a1c93");
  emp_assert(emp::ColorRGB(175,175,144) == "#afaf90");
  emp_assert(emp::ColorRGB(195,30,204) == "#c31ecc");
  emp_assert(emp::ColorRGB(195,0,204) == "#c300cc");
  emp_assert(emp::ColorRGB(195,0,255) == "#c300ff");

  emp_assert(emp::ColorRGB(255,255,255,0.5) == "rgba(255,255,255,0.5)");
  emp_assert(emp::ColorRGB(255,255,255,1.0) == "rgba(255,255,255,1)");
  emp_assert(emp::ColorRGB(26,28,147,0.5) == "rgba(26,28,147,0.5)");
  emp_assert(emp::ColorRGB(26,28,147,1.0) == "rgba(26,28,147,1)");

  emp_assert(emp::ColorHSL(198,100,100) == "hsl(198,100%,100%)");
  emp_assert(emp::ColorHSL(18,0,10) == "hsl(18,0%,10%)");
  emp_assert(emp::ColorHSL(360,6,10) == "hsl(360,6%,10%)");
  emp_assert(emp::ColorHSL(0,6,10) == "hsl(0,6%,10%)");

  emp_assert(emp::ColorHSV(0,0,1.0) == "#ffffff");
  emp_assert(emp::ColorHSV(99.0,0,1.0) == "#ffffff");
  emp_assert(emp::ColorHSV(360.0,0,1.0) == "#ffffff");
  emp_assert(emp::ColorHSV(0,0,0) == "#000000");
  emp_assert(emp::ColorHSV(0.3333*360,1.0,1.0) == "#00ff00");
  emp_assert(emp::ColorHSV(0.6639*360,0.8231,0.5765) == "#1a1c93");
  emp_assert(emp::ColorHSV(0.1667*360,0.1771,0.6863) == "#afaf90");
  emp_assert(emp::ColorHSV(297,0.85,0.8000) == "#c31ecc");
  emp_assert(emp::ColorHSV(0.8260*360,1.0,0.8000) == "#c300cc");
  emp_assert(emp::ColorHSV(286,1.0,1.0) == "#c300ff");

  return 0;
}
