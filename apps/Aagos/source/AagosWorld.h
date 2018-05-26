/// @todo Need to add cyclic environments!

#ifndef AAGOS_WORLD_H
#define AAGOS_WORLD_H

#include "Evolve/NK.h"
#include "Evolve/World.h"
#include "tools/math.h"

#include "AagosOrg.h"

EMP_BUILD_CONFIG( AagosConfig,
  GROUP(WORLD_STRUCTURE, "How should each organism's genome be setup?"),
  VALUE(CHANGE_RATE, size_t, 0, "How many changes to fitness tables each generation?"),
  VALUE(POP_SIZE, size_t, 400, "How many organisms should be in the population?"),
  VALUE(MAX_GENS, size_t, 10000, "How many generations should the runs go for?"),
  VALUE(SEED, int, 0, "Random number seed (0 for based on time)"),
  VALUE(ELITE_COUNT, size_t, 0, "How many organisms should be selected via elite selection?"),
  VALUE(TOURNAMENT_SIZE, size_t, 4, "How many organisms should be chosen for each tournament?"),

  GROUP(GENOME_STRUCTURE, "How should each organism's genome be setup?"),
  VALUE(NUM_BITS, size_t, 64, "Starting number of bits in each organism"),
  VALUE(NUM_GENES, size_t, 64, "Number of genes in each organism"),
  VALUE(GENE_SIZE, size_t, 8, "Size of each gene in each organism"),

  GROUP(MUTATIONS, "Various mutation rates for Aagos"),
  VALUE(GENE_MOVE_PROB, double, 0.001, "Probability of each gene moving each generation"),
  VALUE(BIT_FLIP_PROB, double, 0.01, "Probability of each bit toggling"),
  VALUE(BIT_INS_PROB, double, 0.01, "Probability of a single bit being inserted."),
  VALUE(BIT_DEL_PROB, double, 0.01, "Probability of a single bit being removed."),

  GROUP(OUTPUT, "Output rates for Aagos"),
  VALUE(PRINT_INTERVAL, size_t, 100, "How many updates between prints?")
)


class AagosWorld : public emp::World<AagosOrg> {
private:
  using base_t = emp::World<AagosOrg>;

  AagosConfig & config;
  emp::NKLandscape landscape;

  // Configued values
  size_t num_bits;
  size_t num_genes;
  size_t gene_size;

  // Calculated values
  size_t gene_mask; 

public:
  AagosWorld(AagosConfig & _config, const std::string & world_name="AagosWorld")
    : emp::World<AagosOrg>(world_name)
    , config(_config)
    , landscape(config.NUM_GENES(), config.GENE_SIZE()-1, GetRandom())
    , num_bits(config.NUM_BITS())
    , num_genes(config.NUM_GENES())
    , gene_size(config.GENE_SIZE())
    , gene_mask(emp::MaskLow<size_t>(config.GENE_SIZE()))
  {
    auto fit_fun = [this](AagosOrg & org){
      double fitness = 0.0;
      for (size_t gene_id = 0; gene_id < num_genes; gene_id++) {
        const size_t gene_pos = org.gene_starts[gene_id];
        const size_t gene_val = org.bits.GetUIntAtBit(gene_pos) & gene_mask;
        const size_t tail_bits = num_bits - gene_pos;

        // If a gene runs off the end of the bitstring, loop around to the beginning.
        if (tail_bits < gene_size) {
          gene_val |= (org.bits.GetUInt(0) << tail_bits) & gene_mask;
        }
        fitness += landscape.GetFitness(gene_id, gene_val);
      }
      return fitness;
    };
    SetFitFun(fit_fun);

    // Setup the mutation function.
    std::function<size_t(AagosOrg &, emp::Random &)> mut_fun =
      [this](AagosOrg & org, emp::Random & random) {
        // Do gene moves.
        size_t num_moves = random.GetRandBinomial(org.GetNumGenes(), config.GENE_MOVE_PROB());
        for (size_t m = 0; m < num_moves; m++) {
          size_t gene_id = random.GetUInt(org.GetNumGenes());
          org.gene_starts[gene_id] = random.GetUInt(org.GetNumBits());          
        }

        // Do bit flips.
        size_t num_flips = random.GetRandBinomial(org.GetNumBits(), config.BIT_FLIP_PROB());
        for (size_t m = 0; m < num_flips; m++) {
          const size_t pos = random.GetUInt(org.GetNumBits());
          org.bits[pos] ^= 1;
        }

        // Check on insertions and deletions.
        size_t do_insert = random.P(config.BIT_INS_PROB());
        size_t do_delete = random.P(config.BIT_DEL_PROB());

        // Do insertions.
        if (do_insert) {
          const size_t pos = random.GetUInt(org.GetNumBits());  // Figure out position for insertion.
          org.bits.Resize(org.bits.GetSize() + 1);              // Increase size to make room for insertion.
          emp::BitVector mask(pos, 1);                          // Setup a mask to preserve early bits.
          mask.Resize(org.bits.GetSize());                      // Align mask size.

          // Now build the new string!
          org.bits = (mask & org.bits) | ((org.bits << 1) & ~mask);
          org.bits[pos] = random.P(0.5);                        // Randomize the new bit.

          // Shift any genes that started at pos or later.
          for (auto & x : org.gene_starts) if (x >= pos) x++;
        }

        // Do deletions
        if (do_delete) {
          size_t pos = random.GetUInt(org.GetNumBits());      // Figure out position to delete.
          emp::BitVector mask(pos, 1);                              // Setup a mask to preserve early bits.
          mask.Resize(org.bits.GetSize());                          // Align mask size.
          
          org.bits = (mask & org.bits) | ((org.bits >> 1) & ~mask); // Build the new string!
          org.bits.Resize(org.bits.GetSize() - 1);                  // Decrease size to account for deletion

          // Shift any genes that started at pos or later.
          if (pos == 0) pos = 1;                                    // Adjust position if beginning was deleted.
          for (auto & x : org.gene_starts) if (x >= pos) x--;
        }

        return num_moves + num_flips + do_insert + do_delete;
      };
    SetMutFun( mut_fun );

    SetPopStruct_Mixed(true);
  }
  ~AagosWorld() { ; }

  void Update() {
    landscape.RandomizeStates(GetRandom(), config.CHANGE_RATE());
    base_t::Update();
  }
};

#endif
