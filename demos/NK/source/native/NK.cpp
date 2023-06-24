/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 *  @brief This file explores the template defined in evo::Population.h with an NK landscape.
 */

#include <iostream>

#include "../NKWorld.hpp"

int main(int argc, char* argv[])
{
  NKWorld world;

  // Deal with loading config values via native interface (config file and command line args)
  world.config.Read("NK.cfg");
  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(world.config, std::cout, "NK.cfg", "NK-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

  // Setup and run the world.
  world.Setup();
  world.Run();
}
