/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 *  @brief Taxon object for systematics tracking
 *
 *  Note that this file helps power the Python library phylotrackpy. The main consequence
 *  is that we should generally prefer emp_optional_throw to emp_assert in any circumstance
 *  where user input could trigger an error. emp_assert will trigger a segfault in Python
 *  (killing the whole interpreter), whereas emp_optional_throw will raise a Python exception.
 *

 */

#ifndef EMP_EVOLVE_SYSTEMATICS_TAXON_HPP_INCLUDE
#define EMP_EVOLVE_SYSTEMATICS_TAXON_HPP_INCLUDE


#include "SystematicsData.hpp"

namespace emp{
  /// @brief A Taxon represents a type of organism in a phylogeny.
  /// @param ORG_INFO The information type associated with an organism, used to categorize it.
  ///
  /// Genotypes are the most commonly used Taxon; in general taxa can be anything from a shared
  /// genome sequence, a phenotypic trait, or a even a position in the world (if you want to
  /// track an evolutionary pathway)
  template <typename ORG_INFO, typename DATA_STRUCT = emp::datastruct::no_data>
  class Taxon {
    template <typename ORG, typename SYS_ORG_INFO, typename SYS_DATA_STRUCT>
    friend class Systematics;
  protected:
    using this_t = Taxon<ORG_INFO, DATA_STRUCT>;
    using info_t = ORG_INFO;

    uint64_t id;                ///<  ID for this Taxon (Unique within this Systematics)
    const info_t info;        ///<  Details for the organisms associated within this taxanomic group.
    Ptr<this_t> parent;       ///<  Pointer to parent group (nullptr if injected)
    // std::set<Ptr<this_t> > offspring; ///< Pointers to all immediate offspring taxa
    int num_orgs;          ///<  How many organisms currently exist of this group?
    int num_offspring;     ///<  How many direct offspring groups exist from this one.
    size_t depth;             ///<  How deep in tree is this node? (Root is 0)
    double origination_time;  ///<  When did this taxon first appear in the population?
    double destruction_time;  ///<  When did this taxon leave the population?

    Ptr<this_t> first_child = nullptr;    ///< Pointer to start of linked offspring list
    Ptr<this_t> previous_sibling = nullptr; ///< Pointer to previous sibling in linked list of offspring
    Ptr<this_t> next_sibling = nullptr;     ///< Pointer to next sibling lin linked list of offspring

    [[no_unique_address]] DATA_STRUCT data;         ///< A struct for storing additional information about this taxon

  public:
    using data_t = DATA_STRUCT;

    Taxon(uint64_t _id, const info_t & _info, Ptr<this_t> _parent=nullptr)
     : id (_id), info(_info), parent(_parent)
     , num_orgs(0), num_offspring(0)
     , depth(parent ? (parent->depth+1) : 0)
     , destruction_time(std::numeric_limits<double>::infinity()) { ; }
    // Taxon(const Taxon &) = delete;
    Taxon(const Taxon &) = default; // TODO: Check with Charles about this
    Taxon(Taxon &&) = default;
    Taxon & operator=(const Taxon &) = delete;
    Taxon & operator=(Taxon &&) = default;

    bool operator<(const Taxon & other) const {
      return id < other.GetID();
    }

    /// Get a unique ID for this taxon; IDs are assigned sequentially, so newer taxa have higher IDs.
    uint64_t GetID() const { return id; }

    /// Retrieve the tracked info associated with this Taxon.
    const info_t & GetInfo() const { return info; }

    /// Retrieve a pointer to the parent Taxon.
    Ptr<this_t> GetParent() const { return parent; }
    void NullifyParent() {parent = nullptr;}

    /// Get the number of living organisms currently associated with this Taxon.
    size_t GetNumOrgs() const { return num_orgs; }

    /// Set the number of living organisms currently associated with this Taxon.
    void SetNumOrgs(size_t n) { num_orgs = n; }

    /// Get the total number of organisms that have ever lived associated with this Taxon
    size_t GetTotOrgs() const { 
      if constexpr (data_t::has_total_orgs_t::value) { 
        return data.tot_orgs; 
      } else { 
        emp_optional_throw(false, "GetTotOrgs() called on a Taxon that does not track total organisms. Use cumulative_stats as the DATA_STRUCT template parameter to track total organisms.");
        return 0; 
      } 
    }

    /// Set the total number of organisms that have ever lived associated with this Taxon
    void SetTotOrgs(size_t n) { 
      if constexpr (data_t::has_total_orgs_t::value) { 
        data.tot_orgs = n; 
      } else { 
        emp_optional_throw(false, "SetTotOrgs() called on a Taxon that does not track total organisms. Use cumulative_stats as the DATA_STRUCT template parameter to track total organisms.");
      } 
    }

    /// Get the number of taxa that were produced by organisms from this Taxon.
    size_t GetNumOff() const { return num_offspring; }

    /// Get the number of taxanomic steps since the ancestral organism was injected into the World.
    size_t GetDepth() const { return depth; }

    /// Get data struct associated with this taxon
    data_t & GetData() {return data;}
    /// Get data struct associated with this taxon
    const data_t & GetData() const {return data;}

    /// Get pointers to this taxon's offspring
    std::set<Ptr<this_t> > GetOffspring() {
      std::set<Ptr<this_t> > offspring;
      Ptr<this_t> curr = first_child; 
      while (curr) {
        offspring.insert(curr);
        curr = curr->next_sibling;
      }
      return offspring;
    }

    /// Set this taxon's data struct to the given value
    void SetData(data_t d) {data = d;}

    /// @returns this taxon's origination time
    double GetOriginationTime() const {return origination_time;}
    /// Set this taxon's origination time
    void SetOriginationTime(double time) {origination_time = time;}

    /// @returns this taxon's destruction time
    double GetDestructionTime() const {return destruction_time;}
    /// Sets this taxon's destruction time
    void SetDestructionTime(double time) {destruction_time = time;}

    /// Add a new organism to this Taxon.
    void AddOrg() { 
      ++num_orgs;
      if constexpr (data_t::has_total_orgs_t::value) {
        ++data.tot_orgs;
      }
    }

    void AddToOffspring(Ptr<this_t> offspring_tax) {
      offspring_tax->previous_sibling = nullptr;
      offspring_tax->next_sibling = first_child;
      if (first_child) first_child->previous_sibling = offspring_tax;
      first_child = offspring_tax;
    }

    /// Add a new offspring Taxon to this one.
    void AddOffspring(Ptr<this_t> offspring_tax) {
      ++num_offspring;
      if constexpr (data_t::has_num_offspring_ever_t::value) {
        ++data.num_offspring_ever;
      }
      AddToOffspring(offspring_tax);
      if constexpr (data_t::has_total_offspring_t::value) {
        AddTotalOffspring();
      }
    }

    /// Recursively increment total offspring count for this and all ancestors
    // Should this be protected or private or something?
    void AddTotalOffspring() {
      emp_optional_throw(data_t::has_total_offspring_t::value, "AddTotalOffspring() called on a Taxon that does not track total offspring. Use cumulative_stats_all as the DATA_STRUCT template parameter to track total offspring.");
      if constexpr (data_t::has_total_offspring_t::value) {
        ++data.total_offspring;
        Ptr<this_t> temp_parent = parent;
        while (temp_parent) { // Keep going until we hit root
          temp_parent->data.total_offspring++;
          temp_parent = temp_parent->parent;
        }
      }
    }

    /// Get total number of offspring directly or indirectly
    /// descending from this taxon.
    /// Will only be populated if systematics manager is set up to track
    int GetTotalOffspring() const { 
      if constexpr (data_t::has_total_offspring_t::value) { 
        return data.total_offspring; 
      } 
      emp_optional_throw(data_t::has_total_offspring_t::value, "GetTotalOffspring() called on a Taxon that does not track total offspring. Use cumulative_stats_all as the DATA_STRUCT template parameter to track total offspring.");
      return 0; 
    }

    /// Remove an organism from this Taxon (after it dies).
    /// Removals must return true if the taxon needs to continue; false if it should deactivate.
    bool RemoveOrg() {
      emp_optional_throw(num_orgs > 0, "Removing org from extinct taxon");
      --num_orgs;

      // If we are out of BOTH organisms and offspring, this Taxon should deactivate.
      return num_orgs;
    }

    /// Remove specified taxon from this taxon's offspring list
    void RemoveFromOffspring(Ptr<this_t> offspring_tax) {
      emp_optional_throw(offspring_tax, "Offspring taxon not found in parent's offspring list");
      if (offspring_tax->previous_sibling) {
        offspring_tax->previous_sibling->next_sibling = offspring_tax->next_sibling;
      } else {
        emp_assert(offspring_tax == first_child);
        first_child = offspring_tax->next_sibling;
      }
      if (offspring_tax->next_sibling) {
        offspring_tax->next_sibling->previous_sibling = offspring_tax->previous_sibling;
      }
      offspring_tax->previous_sibling = nullptr;
      offspring_tax->next_sibling = nullptr;
    }

    /// Remove and offspring taxa after its entire sub-tree has died out (pruning)
    bool RemoveOffspring(Ptr<this_t> offspring_tax) {
      emp_optional_throw(num_offspring > 0, "Removing more offspring than exist");
      --num_offspring;
      RemoveFromOffspring(offspring_tax);

      // If we are out of BOTH offspring and organisms, this Taxon should deactivate.
      return num_orgs || num_offspring;
    }

    /// Reduce the total count of extant offspring and recursively do so for
    /// all ancestors (gets called on a taxon's parent when that taxon goes extinct)
    void RemoveTotalOffspring() {
      emp_optional_throw(data_t::has_total_offspring_t::value, "RemoveTotalOffspring() called on a Taxon that does not track total offspring. Use cumulative_stats_all as the DATA_STRUCT template parameter to track total offspring.");      
      if constexpr (data_t::has_total_offspring_t::value) {
        --data.total_offspring;
        Ptr<this_t> temp_parent = parent;
        while (temp_parent) { // Keep going until we hit root
          temp_parent->data.total_offspring--;
          temp_parent = temp_parent->parent;
        }
      }
    }
  };

}

#endif // #ifndef EMP_EVOLVE_SYSTEMATICS_TAXON_HPP_INCLUDE