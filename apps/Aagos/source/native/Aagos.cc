#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"

#include "../AagosOrg.h"
#include "../AagosWorld.h"

int main(int argc, char* argv[])
{
  AagosWorld world;

  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  std::cout << "Hello World!" << std::endl;
}
