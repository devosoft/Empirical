#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Trait.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Trait<int> test_trait("test_trait", "This is a test trait", 42);

  std::cout << test_trait.GetName() << std::endl
            << test_trait.GetDesc() << std::endl;

  return 0;
}
