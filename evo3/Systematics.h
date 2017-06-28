//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Track genotypes, species, clades, or lineages of organisms in a world.
//
//
//  ORG_INFO is usually the genome for an organism, but may have other details like position.
//
//  Options include:
//  * Track lineage yes / no.
//  * Group genotypes yes / no
//  * Group clades (provide a function that determines when a new clade should start)
//  * Prune evolutionary dead-ends (to reduce size of tree).
//  * Backup to file (anything older than a specified level)
//
//
//  Developer notes
//  * Return Ptr<taxon_t> rather than ID to streamline?


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <unordered_map>

#include "../base/Ptr.h"
#include "../tools/map_utils.h"

namespace emp {

  template <typename ORG_INFO>
  class TaxaGroup {
  private:
    const ORG_INFO info;      // Details for the organims associated within this taxanomic group.
    const size_t id;          // Unique ID for this taxonomic group.
    const size_t parent_id;   // ID of parent group (MAX_ID if injected)
    size_t num_orgs;          // How many organisms currently exist of this group?
    size_t tot_orgs;          // How many organisms have ever existed of this group?
    size_t num_offspring;     // How many direct offspring groups exist from this one.
    size_t tot_offspring;     // How many direct offspring have ever existed.

  public:
    TaxaGroup(const ORG_INFO & _info, size_t _id, size_t _pid=0)
    : info(_info), id(_id), parent_id(_pid), num_orgs(0), tot_orgs(0), num_offspring(0) { ; }
    TaxaGroup(const TaxaGroup &) = delete;
    TaxaGroup(TaxaGroup &&) = default;
    TaxaGroup & operator=(const TaxaGroup &) = delete;
    TaxaGroup & operator=(TaxaGroup &&) = default;

    const ORG_INFO & GetInfo() const { return info; }
    size_t GetID() const { return id; }
    size_t GetParentID() const { return parent_id; }
    size_t GetNumOrgs() const { return num_orgs; }
    size_t GetTotOrgs() const { return tot_orgs; }
    size_t GetNumOff() const { return num_offspring; }
    size_t GetTotOff() const { return tot_offspring; }

    void AddOrg() { ++num_orgs; ++tot_orgs; }
    void AddOffspring() { ++num_offspring; ++tot_offspring; }

    // Removals must return true if the taxon needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_assert(num_orgs > 0, id, num_orgs);
      --num_orgs;

      // If we are out of organisms and offspring, this TaxaGroup should deactivate.
      if (num_orgs == 0) return num_offspring;
    }
    bool RemoveOffspring() {
      emp_assert(num_offspring > 0, id);
      --num_offspring;

      // If we are out of offspring and organisms, this TaxaGroup should deactivate.
      if (num_offspring == 0) return num_orgs;
    }
  };

  template <typename ORG_INFO>
  class Systematics {
  private:
    using taxon_t = TaxaGroup<ORG_INFO>;

    std::unordered_map< size_t, Ptr<taxon_t> > taxa_map;
    size_t next_taxon_id;
    constexpr static size_t null_id = 0;

    size_t NextTaxaGroupID() { return ++next_taxon_id; }  // Return a unique TaxaGroup ID.

    void Deactivate(Ptr<taxon_t> taxon) {
      taxa_map.remove(taxon->id);   // Remove taxon from map since it's no longer active.
      Ptr<taxon_t> p_taxon = taxa_map[taxon->parent_id];
      bool p_active = p_taxon->RemoveOffspring();   // Cascade up
      if (p_active == false) Deactivate(p_taxon);
      taxon->Delete();                              // Delete this taxon.
    }

  public:
    Systematics() : taxa_map(), next_taxon_id(0) { ; }
    ~Systematics() {
      for (auto x : taxa_map) x.second.Delete();
      taxa_map.clear();
    }

    const taxon_t & GetTaxaGroup(size_t id) const { return *taxa_map[id]; }

    /// Add information about a newly-injected taxon; return unique taxon id.
    size_t InjectOrg(const ORG_INFO & info) {
      const size_t id = NextTaxaGroupID();
      taxa_map[id] = NewPtr<taxon_t>(info, id);
      taxa_map[id]->AddOrg();
      return id;
    }

    /// Add information about a new organism; return a unique id for the associated taxon.
    size_t AddOrg(size_t parent_id, const ORG_INFO & info) {
      emp_assert(Has(taxa_map, parent_id), parent_id);
      Ptr<taxon_t> p_taxon = taxa_map[parent_id];
      if (p_taxon->GetInfo() == info) {   // Adding another org of this taxon.
        p_taxon->AddOrg();
        return parent_id;
      }
      // This is a new taxon.
      const size_t id = NextTaxaGroupID();
      taxa_map[id] = NewPtr<taxon_t>(info, id, parent_id);
      taxa_map[id]->AddOrg();
      return id;
    }

    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(size_t id) {
      emp_assert(Has(taxa_map, id), id);
      Ptr<taxon_t> taxon = taxa_map[id];
      const bool active = taxon->RemoveOrg();
      if (active == false) Deactivate(taxon);
    }

    /// Climb up a lineage...
    bool ParentID(size_t id) const {
      emp_assert(Has(taxa_map, id), id);
      return *(taxa_map.find(id))->GetParentID();
    }
  };

}

#endif
