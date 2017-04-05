#include <iostream>
#include <fstream>
#include "config/ArgManager.h"

#include "../SimplePDWorld.h"

EMP_BUILD_CONFIG( PDWorldConfig,
  GROUP(DEFAULT, "Default settings for SimplePDWorld"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(r, double, 0.02, "Neighborhood radius, in fraction of world."),
  VALUE(u, double, 0.0025, "cost / benefit ratio"),
  VALUE(N, size_t, 6400, "Number of organisms in the popoulation."),
  VALUE(E, size_t, 5000, "How many epochs should we process?"),
  VALUE(STEP, size_t, 10, "How often should we update data?"),
)

int main(int argc, char * argv[])
{
  PDWorldConfig config;
  config.Read("PDWorld.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "PDWorld.cfg") == false) exit(1);
  if (args.TestUnknown() == false) exit(2);  // If there are leftover args, throw an error.

  const size_t step = config.STEP();
  const double r = config.r();
  const double u = config.u();
  const size_t N = config.N();
  const size_t E = config.E();
  const size_t seed = config.SEED();

  //  std::ofstream f_data(filename_data);
  //  std::ofstream f_dist(filename_dist);

  SimplePDWorld world(r, u, N, E, seed);

  for (size_t e = 0; e < E; e += step) {
    world.Run(step);
    const size_t num_coop = world.CountCoop();
    const size_t num_defect = N - num_coop;
    std::cout << e*step << ' ' << num_coop << ' ' << num_defect << '\n';
  }

  // Print extra info?
  world.PrintNeighborInfo();

  return 0;
}
