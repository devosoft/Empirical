#include "config/ArgManager.h"
#include "evo/NK.h"
#include "evo3/World.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

using BitOrg = emp::BitVector;

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(N, uint32_t, 200, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(POP_SIZE, uint32_t, 1000, "Number of organisms in the popoulation."),
  VALUE(MAX_GENS, uint32_t, 2000, "How many generations should we process?"),
  VALUE(MUT_COUNT, uint32_t, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
  VALUE(TEST, std::string, "TestString", "This is a test string.")
)

struct NKWorld {
  emp::Random random;
  NKConfig config;
  emp::World<BitOrg> world;
  emp::evo::NKLandscape landscape;

  uint32_t N;
  uint32_t K;
  uint32_t POP_SIZE;
  uint32_t MAX_GENS;
  uint32_t MUT_COUNT;

  NKWorld(const std::string & world_name="NKWorld") : world(random, world_name) {
    config.Read("NK.cfg");
  }

  // Run setup after config has been loaded.
  void Setup() {
    world.SetWellMixed(true);
    world.SetCache();

    // Load in config values for easy access.
    N = config.N();
    K = config.K();
    POP_SIZE = config.POP_SIZE();
    MAX_GENS = config.MAX_GENS();
    MUT_COUNT = config.MUT_COUNT();

    landscape.Config(N, K, random);

    // Build a random initial population
    for (uint32_t i = 0; i < POP_SIZE; i++) {
      BitOrg next_org(N);
      for (uint32_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
      world.Inject(next_org);
    }

    // Setup the fitness function.
    std::function<double(BitOrg&)> fit_fun =
      [this](BitOrg & org){ return landscape.GetFitness(org); };
    world.SetFitFun( fit_fun );

    // Setup the mutation function.
    world.SetMutFun( [this](BitOrg & org, emp::Random& random) {
        for (uint32_t m = 0; m < MUT_COUNT; m++) {
          const uint32_t pos = random.GetUInt(N);
          org[pos] = random.P(0.5);
        }
        return true;
      } );
  }

  void Run() {
    std::cout << 0 << " : " << world[0] << " : " << landscape.GetFitness(world[0]) << std::endl;

    // Loop through updates
    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
      // Print current state.
      // for (uint32_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << std::endl;
      // std::cout << std::endl;

      // Keep the best individual.
      world.EliteSelect(1, 1);

      // Run a tournament for the rest...
      world.TournamentSelect(5, POP_SIZE-1);
      world.Update();
      std::cout << (ud+1) << " : " << world[0] << " : " << landscape.GetFitness(world[0]) << std::endl;
      world.MutatePop(1);
    }
  }
};
