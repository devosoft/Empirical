//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Track genotypes, species, clades, or lineages of organisms in a world.
//
//
//  The three arguments to Systematics are:
//    store_active     - Should living organisms' taxa be tracked? (typically yes!)
//    store_ancestors  - Should ancestral organims' taxa be maintained?  (yes for lineages!)
//    store_outside    - Should all dead taxa be maintained? (typically no; it gets BIG!)
//
//  ORG_INFO is usually the genome for an organism, but may have other details like position.
//
//
//  Developer notes
//  * Technically, we don't need to keep the ancestors in a set in order to track a lineage...
//    If we delete all of their descendants they should automaticaly be deleted.
//  * We should provide an option to back up systematics data to a file so that it doesn't all
//    need to be kept in memory, especially if we're only doing post-analysis.


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <ostream>
#include <set>
#include <unordered_set>

#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/set_utils.h"

namespace emp {

  template <typename ORG_INFO>
  class Taxon {
  private:
    using this_t = Taxon<ORG_INFO>;
    using info_t = ORG_INFO;

    size_t id;                //<  ID for this Taxon (Unique within this Systematics)
    const info_t info;        //<  Details for the organims associated within this taxanomic group.
    const Ptr<this_t> parent; //<  Pointer to parent group (nullptr if injected)
    size_t num_orgs;          //<  How many organisms currently exist of this group?
    size_t tot_orgs;          //<  How many organisms have ever existed of this group?
    size_t num_offspring;     //<  How many direct offspring groups exist from this one.

  public:
    Taxon(size_t _id, const info_t & _info, Ptr<this_t> _parent=nullptr)
    : id (_id), info(_info), parent(_parent), num_orgs(0), tot_orgs(0), num_offspring(0) { ; }
    Taxon(const Taxon &) = delete;
    Taxon(Taxon &&) = default;
    Taxon & operator=(const Taxon &) = delete;
    Taxon & operator=(Taxon &&) = default;

    size_t GetID() const { return id; }
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

      // If we are out of BOTH organisms and offspring, this Taxon should deactivate.
      return num_orgs;
    }
    bool RemoveOffspring() {
      emp_assert(num_offspring > 0);
      --num_offspring;

      // If we are out of BOTH offspring and organisms, this Taxon should deactivate.
      return num_orgs || num_offspring;
    }
  };

  template <typename ORG_INFO>
  class Systematics {
  private:
    using taxon_t = Taxon<ORG_INFO>;
    using hash_t = typename Ptr<taxon_t>::hash_t;

    static constexpr bool verbose = true;

    bool store_active;     //< Store all of the currently active taxa?
    bool store_ancestors;  //< Store all of the direct ancestors from living taxa?
    bool store_outside;    //< Store taxa that are extinct with no living descendants?
    bool archive;          //< Set to true if we are supposed to do any archiving of extinct taxa.

    std::unordered_set< Ptr<taxon_t>, hash_t > active_taxa;   //< A set of all living taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > ancestor_taxa; //< A set of all dead, ancestral taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > outside_taxa;  //< A set of all dead taxa w/o descendants.

    size_t next_id;

    void RemoveOffspring(Ptr<taxon_t> taxon) {
      if (!taxon) return;                                // Not tracking this taxon.
      bool still_active = taxon->RemoveOffspring();      // Taxon still active w/ 1 fewer offspring?
      if (still_active == false) {                       // If we're out of offspring, now outside.
        RemoveOffspring( taxon->GetParent() );           // Cascade up to parent taxon.
        if (store_ancestors) ancestor_taxa.erase(taxon); // Clear from ancestors set (if there)
        if (store_outside) outside_taxa.insert(taxon);   // Add to outside set (if tracked)
        else taxon.Delete();                             //  ...or else get rid of it.
      }
    }

    // Mark a taxon extinct if there are no more living members.  There may be descendants.
    void MarkExtinct(Ptr<taxon_t> taxon) {
      emp_assert(taxon);
      emp_assert(taxon->GetNumOrgs() == 0);
      if (store_active) active_taxa.erase(taxon);
      if (!archive) {   // If we don't archive taxa, delete them.
        taxon.Delete();
        return;
      }

      // Otherwise, figure out how we're supposed to store them.
      if (taxon->GetNumOff()) {
        // There are offspring taxa, so store as an ancestor (if we're supposed to).
        if (store_ancestors) ancestor_taxa.insert(taxon);
      } else {
        // The are no offspring; store as an outside taxa or delete.
        RemoveOffspring(taxon->GetParent());            // Recurse to parent.
        if (store_outside) outside_taxa.insert(taxon);  // If we're supposed to store, do so.
        else taxon.Delete();                            // Otherwise delete this taxon.
      }
    }

  public:
    Systematics(bool _active=true, bool _anc=true, bool _all=false)
      : store_active(_active), store_ancestors(_anc), store_outside(_all)
      , archive(store_ancestors || store_outside)
      , active_taxa(), ancestor_taxa(), outside_taxa()
      , next_id(0) { ; }
    Systematics(const Systematics &) = delete;
    Systematics(Systematics &&) = default;
    ~Systematics() {
      for (auto x : active_taxa) x.Delete();
      for (auto x : ancestor_taxa) x.Delete();
      for (auto x : outside_taxa) x.Delete();
      active_taxa.clear();
      ancestor_taxa.clear();
      outside_taxa.clear();
    }

    bool GetStoreActive() const { return store_active; }
    bool GetStoreAncestors() const { return store_ancestors; }
    bool GetStoreOutside() const { return store_outside; }
    bool GetArchive() const { return archive; }

    size_t GetNumActive() const { return  active_taxa.size(); }
    size_t GetNumAncestors() const { return  ancestor_taxa.size(); }
    size_t GetNumOutside() const { return  outside_taxa.size(); }
    size_t GetTreeSize() const { return GetNumActive() + GetNumAncestors(); }
    size_t GetNumTaxa() const { return GetTreeSize() + GetNumOutside(); }

    /// Add information about a new organism; return a pointer for the associated taxon.
    Ptr<taxon_t> AddOrg(const ORG_INFO & info, Ptr<taxon_t> parent=nullptr) {
      if (parent && parent->GetInfo() == info) {   // Adding another org of this taxon.
        emp_assert( Has(active_taxa, parent) );
        parent->AddOrg();
        return parent;
      }
      // Otherwise, this is a new taxon!  If archiving, track the parent.
      Ptr<taxon_t> cur_taxon = NewPtr<taxon_t>(++next_id, info, parent);
      if (store_active) active_taxa.insert(cur_taxon);
      if (parent) parent->AddOffspring();
      cur_taxon->AddOrg();

      return cur_taxon;
    }

    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(Ptr<taxon_t> taxon) {
      emp_assert(taxon);
      // emp_assert(Has(active_taxa, taxon));
      const bool active = taxon->RemoveOrg();
      if (!active) MarkExtinct(taxon);
      return active;
    }

    /// Climb up a lineage...
    Ptr<taxon_t> Parent(Ptr<taxon_t> taxon) const {
      emp_assert(taxon);
      emp_assert(Has(active_taxa, taxon));
      return taxon->GetParent();
    }

    /// Print details about the Systematics manager.
    void PrintStatus(std::ostream & os=std::cout) const {
      os << "Systematics Status:\n";
      os << "Active count:   " << active_taxa.size() << std::endl;
      os << "Ancestor count: " << ancestor_taxa.size() << std::endl;
      os << "Outside count:  " << outside_taxa.size() << std::endl;
    }

    /// Print whole lineage.
    void PrintLineage(Ptr<taxon_t> taxon, std::ostream & os=std::cout) const {
      os << "Lineage:\n";
      while (taxon) {
        os << taxon->GetInfo() << std::endl;
        taxon = taxon->GetParent();
      }
    }

  };

}

#endif
