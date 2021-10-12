#include <iostream>
#include <fstream>
#include "emp/config/ArgManager.hpp"
#include "../SimplePDWorld.hpp"

EMP_BUILD_CONFIG( PDWorldConfig,
  GROUP(DEFAULT, "Default settings for SimplePDWorld"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(r, double, 0.02, "Neighborhood radius, in fraction of world."),
  VALUE(u, double, 0.0025, "cost / benefit ratio"),
  VALUE(N, size_t, 6400, "Number of organisms in the popoulation."),
  VALUE(E, size_t, 5000, "How many epochs should we process?"),
  VALUE(AVE_PAYOFF, bool, 0, "Should we use the total payoff (0), or find the average (1)?"),
  VALUE(STEP, size_t, 10, "How often should we update data?"),
)

int main(int argc, char * argv[])
{
  PDWorldConfig config;
  config.Read("PDWorld.cfg");

  auto args = emp::cl::ArgManager(argc, argv);
  if (args.ProcessConfigOptions(config, std::cout, "PDWorld.cfg") == false) exit(1);
  if (args.TestUnknown() == false) exit(2);  // If there are leftover args, throw an error.

  const size_t seed = config.SEED();
  const double r = config.r();
  const double u = config.u();
  const size_t N = config.N();
  const size_t E = config.E();
  const bool use_ave = config.AVE_PAYOFF();
  const size_t step = config.STEP();

  std::string filename_base = emp::to_string(r,'-',u,'-',N,'-',E,'-',use_ave,'-',seed,".csv");
  std::string filename_data = std::string("data-") + filename_base;
  std::string filename_neigh = std::string("neighborhood_sizes-") + filename_base;

  SimplePDWorld world(r, u, N, E, seed);

  std::ofstream f_data(filename_data);
  std::ofstream f_neigh(filename_neigh);

  // Print extra info?
  world.PrintNeighborInfo(f_neigh);

  f_data << "epoch,num_coop,num_defect\n";
  for (size_t e = 0; e < E; e += step) {
    std::cout << "Epoch = " << e << std::endl;
    world.Run(step);
    const size_t num_coop = world.CountCoop();
    const size_t num_defect = N - num_coop;
    f_data << e << ',' << num_coop << ',' << num_defect << '\n';
  }

  return 0;
}
