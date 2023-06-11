/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file test_gen.cpp
 */

#include "emp/config/command_line.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

#include <iostream>

// Args:
// 1- num orgs
// 2- num criteria
// 3- num fitness tiers

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  if (args.size() != 4) {
    std::cout << "Format: " << args[0] << " [num orgs] [num criteria] [num fitness tiers]"
        << std::endl;
    exit(1);
  }

  size_t num_orgs = emp::from_string<size_t>(args[1]);
  size_t num_fits = emp::from_string<size_t>(args[2]);
  size_t num_tiers = emp::from_string<size_t>(args[3]);

  emp::Random random;

  for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
    if (fit_id > 0) std::cout << ", ";
    std::cout << "Criterion " << fit_id;
  }
  std::cout << std::endl;

  for (size_t org_id = 0; org_id < num_orgs; org_id++) {
    for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
      if (fit_id > 0) std::cout << ", ";
      std::cout << random.GetUInt(num_tiers);
    }
    std::cout << std::endl;
  }
}
