// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/vector.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::vector<int> v(20);

  for (int i = 0; i < 20; i++) {
    v[i] = i * i;
  }

  int total = 0;
  for (int i : v) {
    total += i;
  }

  if (verbose) {
    std::cout << "total = " << total << std::endl;
  }

  emp_assert(total == 2470);

  return 0;
}
