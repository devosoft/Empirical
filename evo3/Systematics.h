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
    const GENOME genome;      // Details for the genome associated with this genotype.
    const size_t id;          // Unique ID for this genotype.
    const size_t parent_id;   // ID of parent genotype (MAX_ID if injected)
    size_t num_orgs;          // How many organisms currently exist of this genotype?
    size_t tot_orgs;          // How many organisms have ever existed of this genotype?
    size_t num_offspring;     // How many direct offspring genotypes exist from this one.
    size_t tot_offspring;     // How many direct offspring have ever existed.

  public:
    Genotype(const GENOME & _gen, size_t _id, size_t _pid=0)
    : genome(_gen), id(_id), parent_id(_pid), num_orgs(0), tot_orgs(0), num_offspring(0) { ; }
    Genotype(const Genotype &) = delete;
    Genotype(Genotype &&) = default;
    Genotype & operator=(const Genotype &) = delete;
    Genotype & operator=(Genotype &&) = default;

    const GENOME & GetGenome() const { return genome; }
    size_t GetID() const { return id; }
    size_t GetParentID() const { return parent_id; }
    size_t GetNumOrgs() const { return num_orgs; }
    size_t GetTotOrgs() const { return tot_orgs; }
    size_t GetNumOff() const { return num_offspring; }
    size_t GetTotOff() const { return tot_offspring; }

    void AddOrg() { ++num_orgs; ++tot_orgs; }
    void AddOffspring() { ++num_offspring; ++tot_offspring; }

    // Removals must return true if the genotype needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_assert(num_orgs > 0, id, num_orgs);
      --num_orgs;

      // If we are out of organisms and offspring, this Genotype should deactivate.
      if (num_orgs == 0) return num_offspring;
    }
    bool RemoveOffspring() {
      emp_assert(num_offspring > 0, id);
      --num_offspring;

      // If we are out of offspring and organisms, this Genotype should deactivate.
      if (num_offspring == 0) return num_orgs;
    }
  };

  template <typename GENOME>
  class Systematics {
  private:
    using genotype_t = Genotype;

    std::unordered_map< size_t, Ptr<genotype_t> > genotype_map;
    size_t next_genotype_id;
    constexpr size_t null_id = 0;

    size_t NextGenotypeID() { return ++next_genotype_id; }  // Return a unique Genotype ID.

    void Deactivate(Genotype & genotype) {
      
    }

  public:
    Systematics() : genotype_map(), next_id(0) { ; }
    ~Systematics() {
      for (auto x : genotype_map) x.second.Delete();
      genotype_map.clear();
    }

    /// Add information about a newly-injected genotype; return unique genotype id.
    size_t InjectOrg(const GENOME & genome) {
      const size_t id = NextGenotypeID();
      genotype_map.emplace(id, {genome, id});
      genotype_map[id].AddOrg();
      return id;
    }

    /// Add information about a new organism; return a unique id for the associated genotype.
    size_t AddOrg(size_t parent_id, const GENOME & genome) {
      emp_assert(Has(genotype_map, parent_id), parent_id);
      Genotype & p_genotype = genotype_map[parent_id];
      if (p_genotype.genome == genome) {   // Adding another org of this genotype.
        p_genotype.AddOrg();
        return parent_id;
      }
      // This is a new genotype.
      const size_t id = NextGenotypeID();
      genotype_map.emplace(id, {genome, id, parent_id});
      genotype_map[id].AddOrg();
      return id;
    }

    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(size_t id) {
      emp_assert(Has(genotype_map, id), id);
      Genotype & genotype = genotype_map[id];
      const bool active = genotype.RemoveOrg();
      if (active == false) Deactivate(genotype);
    }

    /// Climb up a lineage...
    bool ParentID(size_t id) const {
      emp_assert(Has(genotype_map, id), id);
      return genotype_map.find(id)->GetParentID();
    }
  };

}

#endif
