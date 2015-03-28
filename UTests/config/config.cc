#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../config/config.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Config config;
  config.Read("test.cfg");

  if (verbose) {
    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;
  }

  emp_assert(config.RANDOM_SEED() == 333);

  config.RANDOM_SEED(123);

  if (verbose) {
    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;
  }

  emp_assert(config.RANDOM_SEED() == 123);

  return 0;
}
