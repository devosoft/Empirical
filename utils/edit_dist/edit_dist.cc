//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>
#include <vector>

#include "../../config/command_line.h"
#include "../../tools/assert.h"
#include "../../tools/functions.h"
#include "../../tools/sequence_utils.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_flag(args, "-v");

  int N;
  std::cin >> N;
  std::string in1, in2;
  for (int i = 0; i < N; i++) {
    std::cin >> in1 >> in2;
    std::cout << emp::calc_edit_distance(in1, in2) << std::endl;
  }

  return 0;
}
