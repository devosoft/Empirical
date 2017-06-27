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
  class Genotype {
  private:
    GENOME genome;            // Details for the genome associated with this genotype.
    size_t id;                // Unique ID for this genotype.
    size_t parent_id;         // ID of parent genotype (MAX_ID if injected)
    size_t org_count;         // How many organisms currently exist of this genotype?
    size_t tot_count;         // How many organisms have ever existed of this genotype?
    size_t offspring_count;   // How many direct offspring genotypes exist from this one.

  public:
    Genotype(const GENOME & _gen, size_t _id, size_t _pid=0)
    : genome(_gen), id(_id), parent_id(_pid), org_count(0), tot_count(0), offspring_count(0) { ; }
    Genotype(const Genotype &) = delete;
    Genotype(Genotype &&) = default;
    Genotype & operator=(const Genotype &) = delete;
    Genotype & operator=(Genotype &&) = default;

    const GENOME & GetGenome() const { return genome; }
    size_t GetID() const { return id; }
    size_t GetParentID() const { return parent_id; }
    size_t GetOrgCount() const { return org_count; }
    size_t GetTotCount() const { return tot_count; }
    size_t GetOffCount() const { return offspring_count; }
  };

  template <typename GENOME>
  class Systematics {
  private:
    using genotype_t = Genotype;

    std::unordered_map< size_t, Ptr<genotype_t> > genotype_map;
    size_t next_id;
    constexpr size_t null_id = 0;

  public:
    Systematics() : genotype_map(), next_id(1) { ; }
    ~Systematics() {
      for (auto x : genotype_map) x.second.Delete();
      genotype_map.clear();
    }

    /// Add information about a newly-injected genotype; return unique genotype id.
    size_t InjectOrg(const GENOME & genome) {

    }

    /// Add information about a new organism; return a unique id for the associated genotype.
    size_t AddOrg(size_t parent_id, const GENOME & genome) {
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
