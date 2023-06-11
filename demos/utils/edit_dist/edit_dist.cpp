/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file edit_dist.cpp
 *  @brief Load input from standard in that begins with a value "N" and then contains N pairs of strings.
 *
 *  Output will be the edit distances between each string pair.
 */

#include <iostream>
#include <string>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/math/sequence_utils.hpp"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  int N;
  std::cin >> N;
  std::string in1, in2;
  for (int i = 0; i < N; i++) {
    std::cin >> in1 >> in2;
    std::cout << emp::calc_edit_distance(in1, in2) << std::endl;
  }

  return 0;
}
