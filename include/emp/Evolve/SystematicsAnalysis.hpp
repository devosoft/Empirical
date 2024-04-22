/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2018-2023
*/
/**
 *  @file
 *  @brief TODO.
 *
 */

#ifndef EMP_EVOLVE_SYSTEMATICSANALYSIS_HPP_INCLUDE
#define EMP_EVOLVE_SYSTEMATICSANALYSIS_HPP_INCLUDE

#include "../base/Ptr.hpp"

namespace emp {

  /// @returns the taxon with the highest fitness out of any active taxon
  /// in the given systematics manager.
  /// @tparam systematics_t The type of the systematics manager containing the phylogeny to analyze.
  /// @param s the systematics manager to search in. Must have more than 0 active taxa.
  template<typename systematics_t>
  Ptr<typename systematics_t::taxon_t> FindDominant(systematics_t & s) {
    double best = -999999;
    Ptr<typename systematics_t::taxon_t> best_tax = nullptr;
    for (Ptr<typename systematics_t::taxon_t> tax : s.GetActive()) {
      double f = tax->GetData().GetFitness();
      if (f > best) {
        best = f;
        best_tax = tax;
      }
    }
    return best_tax;
  }

  /// Returns the total number of ancestor taxa in \c taxon 's lineage.
  /// Requires that taxon is a member of a systematics manager that
  /// has ancestor storing turned on
  template <typename taxon_t>
  int LineageLength(Ptr<taxon_t> taxon) {
    int count = 0;

    while (taxon) {
      count++;
      taxon = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of times a mutation of type \c type
  /// occurred along \c taxon 's lineage. (Different from CountMuts in
  /// that CountMuts sums them whereas CountMutSteps would count two
  /// simultaneous mutations of the same type as one event)
  /// Assumes each taxon has a struct containing an unordered map
  /// with keys that are strings indicating types of mutations and keys that are numbers
  /// indicating the number of that type of mutation that occurred to make this taxon from
  /// the parent.
  /// @param type string corresponding to a type of mutation.
  /// Must be in the mut_counts dictionary (i.e. the dictionary
  /// passed in when datastruct::mut_landscape_info::RecordMutation was called)
  /// @param taxon a pointer to a taxon to count mutation steps for.
  /// Must have a DATA_TYPE that supports mutation tracking
  /// (e.g. mut_landscape_info)
  template <typename taxon_t>
  int CountMutSteps(Ptr<taxon_t> taxon, std::string type="substitution") {
    int count = 0;

    while (taxon) {
      count += (int)(taxon->GetData().mut_counts[type] > 0);
      taxon = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of times a mutation of the types \c types
  /// that along the given taxon 's lineage. (Different from CountMuts in
  /// that CountMuts sums them whereas CountMutSteps would count two
  /// simultaneous mutations of the same type as one event)
  template <typename taxon_t>
  int CountMutSteps(Ptr<taxon_t> taxon, emp::vector<std::string> types) {
    int count = 0;

    while (taxon) {
      for (std::string type : types) {
        count += (int)(taxon->GetData().mut_counts[type] > 0);
      }
      taxon = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of mutations of type \c type that occurred
  /// along \c taxon 's lineage.
  template <typename taxon_t>
  int CountMuts(Ptr<taxon_t> taxon, std::string type="substitution") {
    int count = 0;

    while (taxon) {
      count += taxon->GetData().mut_counts[type];
      taxon = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of mutations of the types in \c types that occurred
  /// along the given taxon 's lineage.
  template <typename taxon_t>
  int CountMuts(Ptr<taxon_t> taxon, emp::vector<std::string> types) {
    int count = 0;

    while (taxon) {
      for (std::string type : types) {
        count += taxon->GetData().mut_counts[type];
      }
      taxon = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of deleterious mutational steps that occurred
  /// along the given taxon's lineage. (a change from parent to child taxon counts
  /// as a single step, regardless of the number of mutations that happened at
  /// that time point)
  template <typename taxon_t>
  int CountDeleteriousSteps(Ptr<taxon_t> taxon) {
    int count = 0;
    Ptr<taxon_t> parent = taxon->GetParent();

    while (parent) {
      if (taxon->GetData().GetFitness() < parent->GetData().GetFitness()) {
        count++;
      }
      taxon = parent;
      parent = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of changes in phenotype that occurred
  /// along the given taxon's lineage.
  template <typename taxon_t>
  int CountPhenotypeChanges(Ptr<taxon_t> taxon) {
    int count = 0; // Start with current phenotype
    Ptr<taxon_t> parent = taxon->GetParent();

    while (parent) {
      if (taxon->GetData().phenotype != parent->GetData().phenotype) {
        count++;
      }
      taxon = parent;
      parent = taxon->GetParent();
    }

    return count;
  }

  /// Returns the total number of unique phenotypes that occurred
  /// along the given taxon's lineage.
  template <typename taxon_t>
  int CountUniquePhenotypes(Ptr<taxon_t> taxon) {
    std::set<decltype(taxon->GetData().phenotype)> seen;

    while (taxon) {
      seen.insert(taxon->GetData().phenotype);
      taxon = taxon->GetParent();
    }

    return seen.size();
  }

}

#endif // #ifndef EMP_EVOLVE_SYSTEMATICSANALYSIS_HPP_INCLUDE
