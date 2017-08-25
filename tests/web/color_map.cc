//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>

#include "base/assert.h"
#include "config/command_line.h"
#include "web/color_map.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  const auto & test_map = emp::GetHueMap(30);
  const auto & test_map2 = emp::GetHueMap(60);

  for (int i = 0; i < (int) test_map.size(); i++) {
    std::cout << test_map[i] << std::endl;
  }
}
