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

#include <unordered_set>

#include "../base/Ptr.h"
#include "../tools/map_utils.h"

namespace emp {

  template <typename ORG_INFO>
  class TaxaGroup {
  private:
    using this_t = TaxaGroup<ORG_INFO>;
    using info_t = ORG_INFO;

    const info_t info;        // Details for the organims associated within this taxanomic group.
    const Ptr<this_t> parent; // Pointer to parent group (nullptr if injected)
    size_t num_orgs;          // How many organisms currently exist of this group?
    size_t tot_orgs;          // How many organisms have ever existed of this group?
    size_t num_offspring;     // How many direct offspring groups exist from this one.

  public:
    TaxaGroup(const info_t & _info, Ptr<this_t> _parent=nullptr)
    : info(_info), parent(_parent), num_orgs(0), tot_orgs(0), num_offspring(0) { ; }
    TaxaGroup(const TaxaGroup &) = delete;
    TaxaGroup(TaxaGroup &&) = default;
    TaxaGroup & operator=(const TaxaGroup &) = delete;
    TaxaGroup & operator=(TaxaGroup &&) = default;

    const info_t & GetInfo() const { return info; }
    Ptr<this_t> GetParent() const { return parent; }
    size_t GetNumOrgs() const { return num_orgs; }
    size_t GetTotOrgs() const { return tot_orgs; }
    size_t GetNumOff() const { return num_offspring; }

    void AddOrg() { ++num_orgs; ++tot_orgs; }
    void AddOffspring() { ++num_offspring; }

    // Removals must return true if the taxon needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_assert(num_orgs > 0, num_orgs);
      --num_orgs;

      // If we are out of organisms and offspring, this TaxaGroup should deactivate.
      if (num_orgs == 0) return num_offspring;
    }
    bool RemoveOffspring() {
      emp_assert(num_offspring > 0);
      --num_offspring;

      // If we are out of offspring and organisms, this TaxaGroup should deactivate.
      if (num_offspring == 0) return num_orgs;
    }
  };

  template <typename ORG_INFO>
  class Systematics {
  private:
    using taxon_t = TaxaGroup<ORG_INFO>;

    std::unordered_set< Ptr<taxon_t> > active_taxa;
    // @CAO: Save deactiated taxa pointers?

    // Deactivate a taxon when there are not living members AND no living descendents.
    void Deactivate(Ptr<taxon_t> taxon) {
      emp_assert(taxon);
      active_taxa.remove(taxon);                    // Remove taxon from active set.
      Ptr<taxon_t> p_taxon = taxon->GetParent();    // Grab parent taxon to update.
      bool p_active = p_taxon->RemoveOffspring();   // Cascade up
      if (p_active == false) Deactivate(p_taxon);
      taxon->Delete();                              // Delete this taxon.
    }

  public:
    Systematics() : active_taxa() { ; }
    ~Systematics() {
      for (auto x : active_taxa) x.Delete();
      active_taxa.clear();
    }

    /// Add information about a newly-injected taxon; return unique taxon pointer.
    Ptr<taxon_t> InjectOrg(const ORG_INFO & info) {
      Ptr<taxon_t> cur_taxon = NewPtr<taxon_t>(info);
      active_taxa.insert(cur_taxon);
      cur_taxon->AddOrg();
      return cur_taxon;
    }

    /// Add information about a new organism; return a pointer for the associated taxon.
    Ptr<taxon_t> AddOrg(Ptr<taxon_t> parent, const ORG_INFO & info) {
      emp_assert(parent);
      emp_assert( Has(active_taxa, parent) );
      if (parent->GetInfo() == info) {   // Adding another org of this taxon.
        parent->AddOrg();
        return parent;
      }
      // This is a new taxon.
      Ptr<taxon_t> cur_taxon = NewPtr<taxon_t>(info, parent);
      active_taxa.insert(cur_taxon);
      cur_taxon->AddOrg();
      return cur_taxon;
    }

    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(Ptr<taxon_t> taxon) {
      emp_assert(taxon);
      emp_assert(Has(active_taxa, taxon));
      const bool active = taxon->RemoveOrg();
      if (active == false) Deactivate(taxon);
    }

    /// Climb up a lineage...
    Ptr<taxon_t> Parent(Ptr<taxon_t> taxon) const {
      emp_assert(taxon);
      emp_assert(Has(active_taxa, taxon));
      return taxon->GetParent();
    }
  };

}

#endif
