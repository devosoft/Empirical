/// @todo Need to add cyclic environments!

#ifndef AAGOS_WORLD_H
#define AAGOS_WORLD_H

#include "Evolve/NK.h"
#include "Evolve/World.h"

#include "AagosOrg.h"

class AagosWorld : public emp::World<AagosOrg> {
private:
  emp::NKLandscape landscape;

public:
  AagosWorld(size_t num_bits, size_t num_genes, size_t gene_size,
             const std::string & world_name="AagosWorld")
    : emp::World<AagosOrg>(world_name)
    , landscape(num_genes, gene_size-1, GetRandom())
  {
    auto fit_fun = [this](AagosOrg & org){
      double fitness = 0.0;
      for (size_t gene_id = 0; gene_id < num_genes; gene_id++) {
        fitness += landscape.GetFitness(gene_id, org.bits.GetUIntsAtBit(org.gene_starts[gene_id]));
      }
      return fitness;
    };
  }
  ~AagosWorld() { ; }
};

#endif
