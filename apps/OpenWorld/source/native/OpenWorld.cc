// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "base/vector.h"
#include "config/command_line.h"
#include "config/ArgManager.h"

#include "../config.h"
#include "../OpenWorld.h"

int main(int argc, char* argv[])
{
  OpenWorldConfig config;
  config.Read("OpenWorld.cfg", false);
  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "OpenWorld.cfg", "OpenWorld-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  OpenWorld world(config);

  for (size_t ud = 0; ud < config.MAX_GENS(); ud++) {
    world.Update();
    std::cout << "UD: " << ud
              << "  NumOrgs=" << world.GetNumOrgs()
              << std::endl;
  }
}
