#include "emp/config/ArgManager.h"
#include "emp/Evolve/NK.h"
#include "emp/Evolve/World.h"
#include "emp/bits/BitVector.h"
#include "emp/math/Random.h"

using BitOrg = emp::BitVector;

EMP_BUILD_CONFIG( NKConfig,
  GROUP(DEFAULT, "Default settings for NK model"),
  VALUE(K, uint32_t, 10, "Level of epistasis in the NK model"),
  VALUE(N, uint32_t, 100, "Number of bits in each organisms (must be > K)"), ALIAS(GENOME_SIZE),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(MIN_POP_SIZE, uint32_t, 10, "Number of organisms AFTER bottleneck."),
  VALUE(MAX_POP_SIZE, uint32_t, 100, "Number of organisms to trigger bottleneck."),
  VALUE(MAX_GENS, uint32_t, 10000, "How many generations should we process?"),
  VALUE(MUT_COUNT, uint32_t, 3, "How many bit positions should be randomized?"), ALIAS(NUM_MUTS),
)

struct NKWorld : public emp::World<BitOrg> {
  NKConfig config;
  emp::NKLandscape landscape;

  uint32_t N;
  uint32_t K;
  uint32_t MIN_POP_SIZE;
  uint32_t MAX_POP_SIZE;
  uint32_t MAX_GENS;
  uint32_t MUT_COUNT;

  NKWorld(const std::string & world_name="NKWorld")
    : emp::World<BitOrg>(world_name)
  { ; }

  // Run setup after config has been loaded.
  void Setup() {
    SetPopStruct_Grow(false);
    SetCache();

    // Load in config values for easy access.
    N = config.N();
    K = config.K();
    MIN_POP_SIZE = config.MIN_POP_SIZE();
    MAX_POP_SIZE = config.MAX_POP_SIZE();
    MAX_GENS = config.MAX_GENS();
    MUT_COUNT = config.MUT_COUNT();

    emp::Random & random = GetRandom();
    landscape.Config(N, K, random);

    // Build a random initial population
    for (uint32_t i = 0; i < MIN_POP_SIZE; i++) {
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
    SetAutoMutate();

    SetupFitnessFile();
    SetupPopulationFile();
    SetupSystematicsFile();

  }

  void RunStep() {
    const size_t start_orgs = GetNumOrgs();
    if (start_orgs >= MAX_POP_SIZE) {
      DoBottleneck(MIN_POP_SIZE);
    }
    else {
      // Determine number of births for this step.
      size_t num_births = start_orgs;
      if (2 * num_births > MAX_POP_SIZE) num_births = MAX_POP_SIZE - num_births;
      emp::RouletteSelect(*this, num_births);
    }

    Update();
  }

  void Run() {
    std::cout << 0 << " : " << GetNumOrgs() << std::endl;

    // Loop through updates
    for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
      RunStep();
      std::cout << (ud+1) << " : " << GetNumOrgs() << std::endl;
    }
  }
};
