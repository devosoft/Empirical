//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Organisms are bitstrings; fitness is based on the number of times a pattern
//  is repeated.  Combined with ecological pressures (to promote differentiation)
//  this program should pass most open-ended evolution tests.  Instictively,
//  however, these sorts of bitstrings don't seem actually open-ended.

#include <iostream>

#include "../OEEB-World.h"


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

  // After finished, print out the lineage to the command line.
  // world.PrintLineage(0);
}
