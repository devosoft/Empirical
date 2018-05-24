/// @todo Need to add cyclic environments!

#ifndef AAGOS_WORLD_H
#define AAGOS_WORLD_H

#include "Evolve/NK.h"
#include "Evolve/World.h"
#include "tools/math.h"

#include "AagosOrg.h"

EMP_BUILD_CONFIG( AagosConfig,
  GROUP(DEFAULT, "Default settings for Aagos model"),
  VALUE(NUM_BITS, size_t, 64, "Starting number of bits in each organism"),
  VALUE(NUM_GENES, size_t, 64, "Number of genes in each organism"),
  VALUE(GENE_SIZE, size_t, 8, "Size of each gene in each organism"),
  VALUE(CHANGE_RATE, size_t, 0, "How many changes to fitness tables each generation?"),

  VALUE(POP_SIZE, size_t, 400, "How many organisms should be in the population?"),
  VALUE(MAX_GENS, size_t, 10000, "How many generations should the runs go for?"),

  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),

  VALUE(GENE_MOVE_PROB, double, 0.001, "Probability of each gene moving each generation"),
  VALUE(BIT_FLIP_PROB, double, 0.01, "Probability of each bit toggling"),
  VALUE(BIT_INS_PROB, double, 0.01, "Probability of a single bit being inserted."),
  VALUE(BIT_DEL_PROB, double, 0.01, "Probability of a single bit being removed."),

  VALUE(PRINT_INTERVAL, size_t, 100, "How many updates between prints?")
)


class AagosWorld : public emp::World<AagosOrg> {
private:
  using base_t = emp::World<AagosOrg>;

  emp::NKLandscape landscape;

  // Configued values
  size_t num_bits;
  size_t num_genes;
  size_t gene_size;
  size_t change_rate;   ///< How many state-fitnesses should change in the landscape each generation?

  // Calculated values
  size_t gene_mask; 

public:
  AagosWorld(size_t _num_bits, size_t _num_genes, size_t _gene_size, size_t _change_rate=0,
             const std::string & world_name="AagosWorld")
    : emp::World<AagosOrg>(world_name)
    , landscape(_num_genes, _gene_size-1, GetRandom())
    , num_bits(_num_bits)
    , num_genes(_num_genes)
    , gene_size(_gene_size)
    , change_rate(_change_rate)
    , gene_mask(emp::MaskLow<size_t>(gene_size))
  {
    auto fit_fun = [this](AagosOrg & org){
      double fitness = 0.0;
      for (size_t gene_id = 0; gene_id < num_genes; gene_id++) {
        const size_t gene_pos = org.gene_starts[gene_id];
        const size_t gene_val = org.bits.GetUIntAtBit(gene_pos) & gene_mask;
        fitness += landscape.GetFitness(gene_id, gene_val);
      }
      return fitness;
    };
    SetFitFun(fit_fun);

    // Setup the mutation function.
    std::function<size_t(AagosOrg &, emp::Random &)> mut_fun =
      [this](AagosOrg & org, emp::Random & random) {
        constexpr size_t MUT_COUNT = 3;
        size_t num_muts = 0;
        for (uint32_t m = 0; m < MUT_COUNT; m++) {
          const uint32_t pos = random.GetUInt(org.GetNumBits());
          if (random.P(0.5)) {
            org.bits[pos] ^= 1;
            num_muts++;
          }
        }

        // Mutate a gene position?
        if (random.P(0.1)) {
          size_t gene_id = random.GetUInt(org.GetNumGenes());
          org.gene_starts[gene_id] = random.GetUInt(org.GetNumBits());
        }

        return num_muts;
      };
    SetMutFun( mut_fun );

    SetPopStruct_Mixed(true);
  }
  ~AagosWorld() { ; }

  void Update() {
    if (change_rate) landscape.RandomizeStates(GetRandom(), change_rate);
    base_t::Update();
  }
};

#endif
