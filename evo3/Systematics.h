//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Track genotypes, species, clades, and lineages of organisms in a world.
//
//
//  Options include:
//  * Track lineage yes / no.
//  * Group genotypes yes / no
//  * Group clades (provide a function that determines when a new clade should start)
//  * Prune evolutionary dead-ends (to reduce size of tree).
//  * Backup to file (anything older than a specified level)


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <unordered_map>

#include "../base/Ptr.h"

namespace emp {

  template <typename GENOME>
  struct Genotype {
    GENOME genome;            // Details for the genome associated with this genotype.
    size_t id;                // Unique ID for this genotype.
    size_t parent_id;         // ID of parent genotype (MAX_ID if injected)
    size_t org_count;         // How many organisms currently exist of this genotype?
    size_t tot_count;         // How many organisms have ever existed of this genotype?
    size_t offspring_count;   // How many direct offspring genotypes exist from this one.
  };

  template <typename GENOME>
  class Systematics {
  private:
    std::unordered_map< size_t, Ptr<Genotype> > genotype_map;

  public:
    Systematics() : genotype_map() { ; }
    ~Systematics() {
      for (auto x : genotype_map) x.second.Delete();
      genotype_map.clear();
    }

    /// Add information about a new organism; return an id for this new entry.
    size_t AddOrg(size_t parent_id, GENOME & genome) {
    }

    /// Remove an instance of an organism; track when it's gone.
    size_t RemoveOrg(ORG & org) {
    }

    /// Climb up a lineage...
    size_t ParentID(size_t org_id) {
    }
  };

}

#endif
