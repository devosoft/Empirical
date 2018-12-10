/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  GenomeTypeBase.h
 *  @brief Base class for all genome types.
 *
 *  Genomes are generic ways of encoding info for Brains or other aspects of organisms.
 *
 *  Each derived genome class much have a genome_t type indicating a struct with the set of
 *  information it needs to store in organisms.  It must also have the following functions:
 * 
 *    std::string GetClassName()
 *    emp::Config & GetConfig()
 *    void Randomize(emp::Random & random, genome_t & genome)
 *    void Print(std::ostream & os, genome_t & genome) const
 *    void OnBeforeRepro(genome_t & parent_genome)
 *    void OnOffspringReady(genome_t & offspring_genome, genome_t & parent_genome)
 *    void OnInjectReady(genome_t & genome)
 *    void OnBeforePlacement(genome_t & genome)
 *    void OnPlacement(genome_t & genome)
 *    void OnOrgDeath(genome_t & genome)
 */

#ifndef MABE_GENOME_TYPE_BASE_H
#define MABE_GENOME_TYPE_BASE_H

#include <string>

#include "config/config.h"

namespace mabe {

  class GenomeTypeBase {
  private:
    std::string name;
  public:
    GenomeTypeBase() { ; }
    virtual ~GenomeTypeBase() { ; }

    virtual std::string GetClassName() const = 0;
    virtual emp::Config & GetConfig() = 0;

    const std::string & GetName() { return name; }
    void SetName(const std::string & in_name) { name = in_name; }
  };

}

#endif

