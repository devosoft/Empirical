/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  Systematics.h
 *  @brief Track genotypes, species, clades, or lineages of organisms in a world.
 *
 *
 *  @todo Technically, we don't need to keep the ancestors in a set in order to track a lineage...
 *        If we delete all of their descendants they should automaticaly be deleted.
 *  @todo We should provide an option to back up systematics data to a file so that it doesn't all
 *        need to be kept in memory, especially if we're only doing post-analysis.
 *  @todo We should have a systematics interface that will convert organisms into a different
 *        (internal) type to track.  This would allow us to have an arbitrary number of systematics
 *        trackers in world, all of which take an organism, but store different types of data.
 */


#ifndef EMP_EVO_SYSTEMATICS_H
#define EMP_EVO_SYSTEMATICS_H

#include <ostream>
#include <set>
#include <unordered_set>
#include <map>

#include "../base/Ptr.h"
#include "../tools/info_theory.h"
#include "../tools/set_utils.h"
#include "../tools/map_utils.h"
#include "../tools/string_utils.h"

namespace emp {

  /// @brief A Taxon represents a type of organism in a phylogeny.
  /// @param ORG_INFO The information type associated with an organism, used to categorize it.
  ///
  /// Genotypes are the most commonly used Taxon; in general taxa can be anything from a shared
  /// genome sequence, a phenotypic trait, or a even a position in the world (if you want to
  /// track an evolutionary pathway)

  template <typename ORG_INFO>
  class Taxon {
  private:
    using this_t = Taxon<ORG_INFO>;
    using info_t = ORG_INFO;

    size_t id;                ///<  ID for this Taxon (Unique within this Systematics)
    const info_t info;        ///<  Details for the organims associated within this taxanomic group.
    const Ptr<this_t> parent; ///<  Pointer to parent group (nullptr if injected)
    size_t num_orgs;          ///<  How many organisms currently exist of this group?
    size_t tot_orgs;          ///<  How many organisms have ever existed of this group?
    size_t num_offspring;     ///<  How many direct offspring groups exist from this one.
    size_t depth;             ///<  How deep in tree is this node? (Root is 0)
    int origination_time;     ///<  When did this taxon first appear in the population?

  public:
    Taxon(size_t _id, const info_t & _info, Ptr<this_t> _parent=nullptr)
     : id (_id), info(_info), parent(_parent), num_orgs(0), tot_orgs(0), num_offspring(0)
     , depth(parent ? (parent->depth+1) : 0) { ; }
    Taxon(const Taxon &) = delete;
    Taxon(Taxon &&) = default;
    Taxon & operator=(const Taxon &) = delete;
    Taxon & operator=(Taxon &&) = default;

    /// Get a unique ID for this taxon; IDs are assigned sequentially, so newer taxa have higher IDs.
    size_t GetID() const { return id; }

    /// Retrieve the tracked info associated with this Taxon.
    const info_t & GetInfo() const { return info; }

    /// Retrieve a pointer to the parent Taxon.
    Ptr<this_t> GetParent() const { return parent; }

    /// Get the number of living organisms currently associated with this Taxon.
    size_t GetNumOrgs() const { return num_orgs; }

    /// Get the total number of organisms that have ever lived associated with this Taxon
    size_t GetTotOrgs() const { return tot_orgs; }

    /// Get the number of taxa that were produced by organisms from this Taxon.
    size_t GetNumOff() const { return num_offspring; }

    /// Get the number of taxanomic steps since the ancestral organism was injected into the World.
    size_t GetDepth() const { return depth; }

    int GetOriginationTime() const {return origination_time;}
    void SetOriginationTime(int time) {origination_time = time;}

    /// Add a new organism to this Taxon.
    void AddOrg() { ++num_orgs; ++tot_orgs; }

    /// Add a new offspring Taxon to this one.
    void AddOffspring() { ++num_offspring; }

    /// Remove an organism from this Taxon (after it dies).
    /// Removals must return true if the taxon needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_assert(num_orgs > 0, num_orgs);
      --num_orgs;

      // If we are out of BOTH organisms and offspring, this Taxon should deactivate.
      return num_orgs;
    }

    /// Remove and offspring taxa after its entire sub-tree has died out (pruning)
    bool RemoveOffspring() {
      emp_assert(num_offspring > 0);
      --num_offspring;

      // If we are out of BOTH offspring and organisms, this Taxon should deactivate.
      return num_orgs || num_offspring;
    }
  };

  /// @brief A tool to track phylogenetic relationships among organisms.
  /// The systematics class tracks the relationships among all organisms based on the INFO_TYPE
  /// provided.  If an offspring has the same value for INFO_TYPE as its parent, it is grouped into
  /// the same taxon.  Otherwise a new Taxon is created and the old one is used as its parent in
  /// the phylogeny.  If the provided INFO_TYPE is the organsism's genome, a traditional phylogeny
  /// is formed, with genotypes.  If the organism's behavior/task set is used, then organisms are
  /// grouped by phenotypes.  If the organsims's position is used, the evolutionary path through
  /// space is tracked.  Any other aspect of organisms can be tracked this way as well.
  template <typename ORG_INFO>
  class Systematics {
  private:
    using taxon_t = Taxon<ORG_INFO>;
    using hash_t = typename Ptr<taxon_t>::hash_t;

    bool store_active;     ///< Store all of the currently active taxa?
    bool store_ancestors;  ///< Store all of the direct ancestors from living taxa?
    bool store_outside;    ///< Store taxa that are extinct with no living descendants?
    bool archive;          ///< Set to true if we are supposed to do any archiving of extinct taxa.

    std::unordered_set< Ptr<taxon_t>, hash_t > active_taxa;   ///< A set of all living taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > ancestor_taxa; ///< A set of all dead, ancestral taxa.
    std::unordered_set< Ptr<taxon_t>, hash_t > outside_taxa;  ///< A set of all dead taxa w/o descendants.

    // Stats about active taxa... (totals are across orgs, not taxa)
    size_t org_count;           ///< How many organisms are currently active?
    size_t total_depth;         ///< Sum of taxa depths for calculating average.
    size_t num_roots;           ///< How many distint injected ancestors are currently in population?

    size_t next_id;             ///< What ID value should the next new taxon have?
    mutable Ptr<taxon_t> mrca;  ///< Most recent common ancestor in the population.

    /// Called wheneven a taxon has no organisms AND no descendants.
    void Prune(Ptr<taxon_t> taxon);

    /// Called when an offspring taxa has been deleted.
    void RemoveOffspring(Ptr<taxon_t> taxon);

    /// Called when there are no more living members of a taxon.  There may be descendants.
    void MarkExtinct(Ptr<taxon_t> taxon);

  public:
    /**
     * Contructor for Systematics; controls what information should be stored.
     * @param store_active     Should living organisms' taxa be tracked? (typically yes!)
     * @param store_ancestors  Should ancestral organims' taxa be maintained?  (yes for lineages!)
     * @param store_outside    Should all dead taxa be maintained? (typically no; it gets BIG!)
     */
    Systematics(bool _active=true, bool _anc=true, bool _all=false)
      : store_active(_active), store_ancestors(_anc), store_outside(_all)
      , archive(store_ancestors || store_outside)
      , active_taxa(), ancestor_taxa(), outside_taxa()
      , org_count(0), total_depth(0), num_roots(0), next_id(0)
      , mrca(nullptr) { ; }
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

    /// Are we storing all taxa that are still alive in the population?
    bool GetStoreActive() const { return store_active; }

    /// Are we storing all taxa that are the ancestors of living organims in the population?
    bool GetStoreAncestors() const { return store_ancestors; }

    /// Are we storing all taxa that have died out, as have all of their descendants.
    bool GetStoreOutside() const { return store_outside; }

    /// Are we storing any taxa types that have died out?
    bool GetArchive() const { return archive; }

    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetActive() const { return active_taxa; }
    const std::unordered_set< Ptr<taxon_t>, hash_t > & GetAncestors() const { return ancestor_taxa; }


    /// How many taxa are still active in the population?
    size_t GetNumActive() const { return active_taxa.size(); }

    /// How many taxa are ancestors of living organisms (but have died out themselves)?
    size_t GetNumAncestors() const { return ancestor_taxa.size(); }

    /// How many taxa are stored that have died out, as have their descendents?
    size_t GetNumOutside() const { return outside_taxa.size(); }

    /// How many taxa are in the current phylogeny?
    size_t GetTreeSize() const { return GetNumActive() + GetNumAncestors(); }

    /// How many taxa are stored in total?
    size_t GetNumTaxa() const { return GetTreeSize() + GetNumOutside(); }

    /// How many living organisms are currently being tracked?
    size_t GetTotalOrgs() const { return org_count; }

    /// How many independent trees are being tracked?
    size_t GetNumRoots() const { return num_roots; }

    /// What is the average phylogenetic depth of organisms in the population?
    double GetAveDepth() const { return ((double) total_depth) / (double) org_count; }

    /** From (Faith 1992, reviewed in Winters et al., 2013), phylogenetic diversity is 
     * "Calculated as the sum of branch lengths between root and tips for a community".
     */ 
    double GetPhylogeneticDiversity() const {
      int mrca_depth = GetMRCADepth();
      if (mrca_depth != -1) {
        return total_depth - org_count*mrca_depth;
      }

      // TODO: Track MRCA for multiple trees.
      return total_depth;
    }
 
    /** This is a metric of how distinct @param tax is from the rest of the population.
     * 
     * (From Vane-Wright et al., 1991; reviewed in Winter et al., 2013)
    */
    double GetTaxonDistinctiveness(Ptr<taxon_t> tax) const {return 1/GeDistanceToRoot(tax);}

    /** This metric (from Isaac, 2007; reviewd in Winter et al., 2013) measures how
     * distinct @param tax is from the rest of the population, weighted for the amount of
     * unique evolutionary history that it represents. 
     * 
     * To quantify length of evolutionary history, this method needs @param time: the current
     * time, in whatever units time is being measured in when taxa are added to the systematics
     * manager.
    */
    double GetEvolutionaryDistinctiveness(Ptr<taxon_t> tax, int time) const {
 
      int total = 0;
      int divisor = 1; // Number of extant taxa this will split into

      Ptr<taxon_t> test_taxon = tax->GetParent();
      int depth = time - tax->GetOriginationTime();
      time = tax->GetOriginationTime();
      while (test_taxon) {
        emp_assert(time != -1, time);
        depth += time - test_taxon->GetOriginationTime();
        time = test_taxon->GetOriginationTime();
        if (test_taxon == mrca || !test_taxon) {
          total += depth/divisor;
          return total;
        } else if (test_taxon.GetNumOff() > 1) {
          total += depth/divisor;
          depth = 0;
          divisor = test_taxon.GetNumOff();
        }
        test_taxon = test_taxon->GetParent();
      }
    }

    /** Calculates mean pairwise distance between extant taxa (Webb and Losos, 2000).
     * This measurement is also called Average Taxonomic Diversity (Warwick and Clark, 1998)
     * (for demonstration of equivalence see Tucker et al, 2016). This measurment tells
     * you about the amount of distinctness in the community as a whole.
     * 
     * @param branch_only only counts distance in terms of nodes that represent a branch
     * between two extant taxa (poentially useful for comparison to biological data, where
     * non-branching nodes generally cannot be inferred).
     * */
    double GetMeanPairwiseDistance(bool branch_only=false) {
      
      emp::vector<int> dists;
      
      std::map< Ptr<taxon_t>, emp::vector<emp::vector<int>> > curr_pointers;
      std::map< Ptr<taxon_t>, emp::vector<emp::vector<int>> > next_pointers;

      
      for (Ptr<taxon_t> tax : active_taxa) {
        curr_pointers[tax] = emp::vector<emp::vector<int>>({{0}});
      }

      // std::cout << "Starting curr_pointers size: " << curr_pointers.size() << std::endl;

      while (curr_pointers.size() > 0) {
        for (auto & tax : curr_pointers) {
          bool alive = tax.first->GetNumOrgs() > 0;
          // std::cout << tax.first << " has " << to_string(tax.second) << "and is waiting for " << tax.first->GetNumOff() + int(alive) << std::endl;
          if ( tax.second.size() < tax.first->GetNumOff() + int(alive)) {
            if (Has(next_pointers, tax.first)) {
              // In case an earlier iteration added this node to next_pointers
              for (auto vec : tax.second) {
                next_pointers[tax.first].push_back(vec);
              }
            } else {
              next_pointers[tax.first] = curr_pointers[tax.first];
            } 
            continue;
          }
          emp_assert(tax.first->GetNumOff() + int(alive) == tax.second.size(), tax.first->GetNumOff(), alive, to_string(tax.second), tax.second.size());

          // Okay, things should have just met up. Let's compute the distances
          // between everything that just met.

          if (tax.second.size() > 1) {
       
            for (size_t i = 0; i < tax.second.size(); i++ ) {
              for (size_t j = i+1; j < tax.second.size(); j++) {
                for (int disti : tax.second[i]) {
                  for (int distj : tax.second[j]) {
                    // std::cout << "Adding " << disti << " and " << distj << std::endl;
                    dists.push_back(disti+distj);
                  }
                }
              }
            }
          }
          // std::cout << "dists " << to_string(dists) << std::endl; 
          // Increment distances and stick them in new vector
          emp::vector<int> new_dist_vec; 
          for (auto & vec : tax.second) {
            for (int el : vec) {
              new_dist_vec.push_back(el+1);
            }
          }

          // std::cout << "new_dist_vec " << to_string(new_dist_vec) << std::endl; 

          next_pointers.erase(tax.first);

          Ptr<taxon_t> test_taxon = tax.first->GetParent();
          while (test_taxon && test_taxon->GetNumOff() == 1 && test_taxon->GetNumOrgs() < 0) {
            if (!branch_only) {
              for (size_t i = 0; i < new_dist_vec.size(); i++){
                new_dist_vec[i]++;
              }
            }
            test_taxon = test_taxon->GetParent();
          }

          if (!test_taxon) {
            continue;
          } else if (!Has(next_pointers, test_taxon)) {
            next_pointers[test_taxon] = emp::vector<emp::vector<int> >({new_dist_vec});
          } else {
            next_pointers[test_taxon].push_back(new_dist_vec);
          }
        }
        curr_pointers = next_pointers;
        next_pointers.clear();
        // std::cout << curr_pointers.size() << std::endl;
      }

      if (dists.size() != (active_taxa.size()*(active_taxa.size()-1))/2) {
        // The tree is not connected
        // Technically this means the mean distance is infinite.
        return INFINITY;
      }

      double total = 0;
      for (int dist : dists) {
        total += dist;
      }

      // std::cout << "Total: " << total << "Dists: " << dists.size() << std::endl;

      return total/dists.size();

    }


    /** Counts the total number of ancestors between @param tax and MRCA, if there is one. If
     * There is no common ancestor, distance to the root of this tree is calculated instead.
    */
    int GetDistanceToRoot(Ptr<taxon_t> tax) const {
      // Now, trace the line of descent, updating the candidate as we go.
      GetMRCA();

      int depth = 0;
      Ptr<taxon_t> test_taxon = tax->GetParent();
      while (test_taxon) {
        if (test_taxon == mrca || !test_taxon) {
          return depth;
        }
        depth++;
        test_taxon = test_taxon->GetParent();
      }
    }

    /** Counts the number of branching points leading to multiple extant taxa 
     * between @param tax and the most-recent common ancestor (or the root of its subtree,
     * if no MRCA exists). This is useful because a lot
     * of stats for phylogenies are designed for phylogenies reconstructed from extant taxa.
     * These phylogenies generally only contain branching points, rather than every ancestor
     * along the way to the current taxon.*/
    int GetBranchesToRoot(Ptr<taxon_t> tax) const {
      GetMRCA();

      int depth = 0;
      Ptr<taxon_t> test_taxon = tax->GetParent();
      while (test_taxon) {
        if (test_taxon == mrca || !test_taxon) {
          return depth;
        } else if (test_taxon.GetNumOff() > 1) {
          depth++;
        }
        test_taxon = test_taxon->GetParent();
      }
    }

    /// Request a pointer to the Most-Recent Common Ancestor for the population.
    Ptr<taxon_t> GetMRCA() const;

    /// Request the depth of the Most-Recent Common Ancestor; return -1 for none.
    int GetMRCADepth() const;

    /// Add information about a new organism, including its stored info and parent's taxon;
    /// return a pointer for the associated taxon.
    Ptr<taxon_t> AddOrg(const ORG_INFO & info, Ptr<taxon_t> cur_taxon=nullptr, int update=-1);

    /// Remove an instance of an organism; track when it's gone.
    bool RemoveOrg(Ptr<taxon_t> taxon);

    /// Climb up a lineage...
    Ptr<taxon_t> Parent(Ptr<taxon_t> taxon) const;

    /// Print details about the Systematics manager.
    void PrintStatus(std::ostream & os=std::cout) const;

    /// Print whole lineage.
    void PrintLineage(Ptr<taxon_t> taxon, std::ostream & os=std::cout) const;

    /// Calculate the genetic diversity of the population.
    double CalcDiversity();

    /// Are we storing all taxa that are still alive in the population?
    void SetStoreActive(bool new_val) { store_active = new_val; }

    /// Are we storing all taxa that are the ancestors of living organims in the population?
    void SetStoreAncestors(bool new_val) { store_ancestors = new_val; }

    /// Are we storing all taxa that have died out, as have all of their descendants.
    void SetStoreOutside(bool new_val) { store_outside = new_val; }

    /// Are we storing any taxa types that have died out?
    void SetArchive(bool new_val) { archive = new_val; }


  };

  // =============================================================
  // ===                                                       ===
  // ===  Out-of-class member function definitions from above  ===
  // ===                                                       ===
  // =============================================================

  // Should be called wheneven a taxon has no organisms AND no descendants.
  template <typename ORG_INFO>
  void Systematics<ORG_INFO>::Prune(Ptr<taxon_t> taxon) {
    RemoveOffspring( taxon->GetParent() );           // Notify parent of the pruning.
    if (store_ancestors) ancestor_taxa.erase(taxon); // Clear from ancestors set (if there)
    if (store_outside) outside_taxa.insert(taxon);   // Add to outside set (if tracked)
    else taxon.Delete();                             //  ...or else get rid of it.
  }

  template <typename ORG_INFO>
  void Systematics<ORG_INFO>::RemoveOffspring(Ptr<taxon_t> taxon) {
    if (!taxon) { num_roots--; return; }               // Offspring was root; remove and return.
    bool still_active = taxon->RemoveOffspring();      // Taxon still active w/ 1 fewer offspring?
    if (!still_active) Prune(taxon);                   // If out of offspring, remove from tree.

    // If the taxon is still active AND the is the current mrca AND now has only one offspring,
    // clear the MRCA for lazy re-evaluation later.
    else if (taxon == mrca && taxon->GetNumOff() == 1) mrca = nullptr;
  }

  // Mark a taxon extinct if there are no more living members.  There may be descendants.
  template <typename ORG_INFO>
  void Systematics<ORG_INFO>::MarkExtinct(Ptr<taxon_t> taxon) {
    emp_assert(taxon);
    emp_assert(taxon->GetNumOrgs() == 0);

    if (store_active) active_taxa.erase(taxon);
    if (!archive) {   // If we don't archive taxa, delete them.
      taxon.Delete();
      return;
    }

    if (store_ancestors) ancestor_taxa.insert(taxon);  // Move taxon to ancestors...
    if (taxon->GetNumOff() == 0) Prune(taxon);         // ...and prune from there if needed.
  }


  // Request a pointer to the Most-Recent Common Ancestor for the population.
  template <typename ORG_INFO>
  Ptr<Taxon<ORG_INFO>> Systematics<ORG_INFO>::GetMRCA() const {
    if (!mrca && num_roots == 1) {  // Determine if we need to calculate the MRCA.
      // First, find a candidate among the living taxa.  Only taxa that have one offsrping
      // can be on the line-of-descent to the MRCA, so anything else is a good start point.
      // There must be at least one!  Stop as soon as we find a candidate.
      Ptr<taxon_t> candidate(nullptr);
      for (auto x : active_taxa) {
        if (x->GetNumOff() != 1) { candidate = x; break; }
      }

      // Now, trace the line of descent, updating the candidate as we go.
      Ptr<taxon_t> test_taxon = candidate->GetParent();
      while (test_taxon) {
        emp_assert(test_taxon->GetNumOff() >= 1);
        if (test_taxon->GetNumOff() > 1) candidate = test_taxon;
        test_taxon = test_taxon->GetParent();
      }
      mrca = candidate;
    }
    return mrca;
  }

  // Request the depth of the Most-Recent Common Ancestor; return -1 for none.
  template <typename ORG_INFO>
  int Systematics<ORG_INFO>::GetMRCADepth() const {
    GetMRCA();
    if (mrca) return mrca->GetDepth();
    return -1;
  }

  // Add information about a new organism, including its stored info and parent's taxon;
  // return a pointer for the associated taxon.
  template <typename ORG_INFO>
  Ptr<Taxon<ORG_INFO>> Systematics<ORG_INFO>::AddOrg(const ORG_INFO & info, Ptr<taxon_t> cur_taxon, int update) {
    emp_assert( !cur_taxon || Has(active_taxa, cur_taxon));

    // Update stats
    org_count++;                  // Keep count of how many organisms are being tracked.

    // If this organism needs a new taxon, build it!
    if (!cur_taxon || cur_taxon->GetInfo() != info) {
      auto parent_taxon = cur_taxon;                               // Provided taxon is parent.
      if (!parent_taxon) {                                         // No parnet -> NEW tree
        num_roots++;                                               // ...track extra root.
        mrca = nullptr;                                            // ...nix old common ancestor
      }
      cur_taxon = NewPtr<taxon_t>(++next_id, info, parent_taxon);  // Build new taxon.
      if (store_active) active_taxa.insert(cur_taxon);             // Store new taxon.
      if (parent_taxon) parent_taxon->AddOffspring();              // Track tree info.
    }

    cur_taxon->AddOrg();                    // Record the current organism in its taxon.
    cur_taxon->SetOriginationTime(update);
    total_depth += cur_taxon->GetDepth();   // Track the total depth (for averaging)
    return cur_taxon;                       // Return the taxon used.
  }

  // Remove an instance of an organism; track when it's gone.
  template <typename ORG_INFO>
  bool Systematics<ORG_INFO>::RemoveOrg(Ptr<taxon_t> taxon) {
    emp_assert(taxon);

    // Update stats
    org_count--;
    total_depth -= taxon->GetDepth();

    // emp_assert(Has(active_taxa, taxon));
    const bool active = taxon->RemoveOrg();
    if (!active) MarkExtinct(taxon);

    return active;
  }

  // Climb up a lineage...
  template <typename ORG_INFO>
  Ptr<Taxon<ORG_INFO>> Systematics<ORG_INFO>::Parent(Ptr<taxon_t> taxon) const {
    emp_assert(taxon);
    emp_assert(Has(active_taxa, taxon));
    return taxon->GetParent();
  }

  // Print details about the Systematics manager.
  template <typename ORG_INFO>
  void Systematics<ORG_INFO>::PrintStatus(std::ostream & os) const {
    os << "Systematics Status:\n";
    os << " store_active=" << store_active
       << " store_ancestors=" << store_ancestors
       << " store_outside=" << store_outside
       << " archive=" << archive
       << " next_id=" << next_id
       << std::endl;
    os << "Active count:   " << active_taxa.size();
    for (const auto & x : active_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << ((bool) x->GetParent()) << "]";
    }
    os << std::endl;

    os << "Ancestor count: " << ancestor_taxa.size();
    for (const auto & x : ancestor_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << ((bool) x->GetParent()) << "]";
    }
    os << std::endl;

    os << "Outside count:  " << outside_taxa.size();
    for (const auto & x : outside_taxa) {
      os << " [" << x->GetID() << "|" << x->GetNumOrgs() << "," << x->GetNumOff() << "|"
         << ((bool) x->GetParent()) << "]";
    }
    os << std::endl;
  }

  // Print whole lineage.
  template <typename ORG_INFO>
  void Systematics<ORG_INFO>::PrintLineage(Ptr<taxon_t> taxon, std::ostream & os) const {
    os << "Lineage:\n";
    while (taxon) {
      os << taxon->GetInfo() << std::endl;
      taxon = taxon->GetParent();
    }
  }

  // Calculate the genetic diversity of the population.
  template <typename ORG_INFO>
  double Systematics<ORG_INFO>::CalcDiversity() {
    return emp::Entropy(active_taxa, [](Ptr<taxon_t> x){ return x->GetNumOrgs(); }, org_count);
  }

}

#endif
