/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  GenomeWrapper.h
 *  @brief A template wrapper for a genome object that will simplify access to optional traits.
 *
 *  Genomes are generic ways of encoding info for Brains or other aspects of organisms.
 *
 *  Each genome class must contain any heritable material for an organism.  It must also have any
 *  of the following functions:
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

#ifndef MABE_GENOME_WRAPPER_H
#define MABE_GENOME_WRAPPER_H

#include <string>
#include <utility>

#include "base/macros.h"
#include "config/config.h"
#include "config/config_utils.h"
#include "meta/ConceptWrapper.h"
#include "meta/meta.h"

namespace mabe {

  EMP_BUILD_CONCEPT( GenomeWrapper, GenomeBase,
    OPTIONAL_FUN(GetClassName, "Unnamed Genome", std::string),
    OPTIONAL_FUN(GetConfig, emp::GetEmptyConfig(), emp::Config &),
    OPTIONAL_FUN(Randomize, false, bool, emp::Random &),
    OPTIONAL_FUN(Print, false, bool),
    OPTIONAL_FUN(OnBeforeRepro, , void),                  // Genome about to be reproduced.
    OPTIONAL_FUN(OnOffspringReady, , void, GenomeBase &),     // Genome offspring; arg is parent genome
    OPTIONAL_FUN(OnInjectReady, , void),                  // Genome about to be injected.
    OPTIONAL_FUN(OnBeforePlacement, , void),              // Genome about to be placed
    OPTIONAL_FUN(OnPlacement, , void),                    // Genome just placed.
    OPTIONAL_FUN(OnOrgDeath, , void)                      // Genome about to die.
  );


}

#endif

