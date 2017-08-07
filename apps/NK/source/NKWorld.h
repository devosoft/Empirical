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

struct NKWorld : public emp::World<BitOrg> {
  NKConfig config;
  emp::evo::NKLandscape landscape;

  uint32_t N;
  uint32_t K;
  uint32_t POP_SIZE;
  uint32_t MAX_GENS;
  uint32_t MUT_COUNT;

  NKWorld(const std::string & world_name="NKWorld")
  : emp::World<BitOrg>(world_name) {
  }

  // Run setup after config has been loaded.
  void Setup() {
    SetWellMixed(true);
    SetCache();

    // Load in config values for easy access.
    N = config.N();
    K = config.K();
    POP_SIZE = config.POP_SIZE();
    MAX_GENS = config.MAX_GENS();
    MUT_COUNT = config.MUT_COUNT();

    emp::Random & random = GetRandom();
    landscape.Config(N, K, random);

    // Build a random initial population
    for (uint32_t i = 0; i < POP_SIZE; i++) {
      BitOrg next_org(N);
      for (uint32_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
      Inject(next_org);
    }

    // Setup the fitness function.
    std::function<double(BitOrg&)> fit_fun =
      [this](BitOrg & org){ return landscape.GetFitness(org); };
    SetFitFun( fit_fun );

    // Setup the mutation function.
    std::function<size_t(BitOrg &, emp::Random &)> mut_fun =
      [this](BitOrg & org, emp::Random & random) {
        size_t num_muts = 0;
        for (uint32_t m = 0; m < MUT_COUNT; m++) {
          const uint32_t pos = random.GetUInt(N);
          if (random.P(0.5)) {
            org[pos] ^= 1;
            num_muts++;
          }
        }
        return num_muts;
      };
    SetMutFun( mut_fun );
  }

  void RunStep() {
    // Do mutations on the population.
    MutatePop(1);

    // Keep the best individual.
    EliteSelect(1, 1);

    // Run a tournament for the rest...
    TournamentSelect(5, POP_SIZE-1);
    Update();
  }

  void Run() {
    std::cout << 0 << " : " << *pop[0] << " : " << landscape.GetFitness(*pop[0]) << std::endl;

    // Loop through updates
    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
      RunStep();
      std::cout << (ud+1) << " : " << *pop[0] << " : " << landscape.GetFitness(*pop[0]) << std::endl;
    }
  }
};
