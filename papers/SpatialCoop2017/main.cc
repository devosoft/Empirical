#include <iostream>
#include "../../config/ArgManager.h"

#include "SimplePDWorld.h"

EMP_BUILD_CONFIG( PDWorldConfig,
  GROUP(DEFAULT, "Default settings for SimplePDWorld"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(r, double, 0.02, "Neighborhood radius, in fraction of world."),
  VALUE(u, double, 0.0025, "cost / benefit ratio"),
  VALUE(N, size_t, 6400, "Number of organisms in the popoulation."),
  VALUE(E, size_t, 5000, "How many epochs should we process?"),
)

int main(int argc, char * argv[])
{
  PDWorldConfig config;
  config.Read("PDWorld.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "PDWorld.cfg") == false) exit(1);
  if (args.TestUnknown() == false) exit(2);  // If there are leftover args, throw an error.

  SimplePDWorld world(config.r(), config.u(), config.N(), config.E(), config.SEED());
  world.Run();

  // Print extra info?
  world.PrintNeighborInfo();

  return 0;
}
