//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class maintains all of the fitness data for a population of organisms.


#ifndef SELECTION_DATA_H
#define SELECTION_DATA_H

#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <unordered_map>

#include "../../source/base/vector.h"
#include "../../source/tools/BitVector.h"
#include "../../source/tools/File.h"
#include "../../source/tools/random_utils.h"
#include "../../source/tools/vector_utils.h"

class SelectionData {
private:
  bool verbose = false;                        ///< Should we print extra info while processing?

  using pop_fit_t = emp::vector<double>;       ///< Type for individual fitnesses for a single function.
  emp::vector< pop_fit_t > org_chart;          ///< Chart of all fitnesses for each organism.
  emp::vector< pop_fit_t > fitness_chart;      ///< Chart of all fitnesses for each function.
  emp::vector< pop_fit_t > orig_org_chart;     ///< Backup of org fitness chart to fix on reset.
  emp::vector< pop_fit_t > orig_fitness_chart; ///< Backup of fitness chart of orgs to fix on reset.

  /// Cache the probabilities for subsets of populations.
  std::unordered_map<emp::BitVector, emp::vector<double>> prob_cache;

  emp::BitVector is_dominated;            ///< Is org guaranteed to have a chance?
  emp::BitVector is_active;               ///< Is org non-dominated AND not a duplicate.

  emp::BitVector is_discrim;              ///< Is criterium discrimanatory.

  struct OrgInfo {
    emp::vector< size_t > dup_ids = {};   ///< What OTHER ids are lumped in with this one?
    double select_prob = 0.0;             ///< What is the probability of this group being picked?

    size_t GetSize() { return 1 + dup_ids.size(); }
    double GetWeight() { return 1.0 + (double) dup_ids.size(); }
  };

  struct CriterionInfo {
    emp::vector< size_t > dup_ids = {};

    size_t GetSize() { return 1 + dup_ids.size(); }
    double GetWeight() { return 1.0 + (double) dup_ids.size(); }
  };

  emp::vector< OrgInfo > org_info;
  emp::vector< CriterionInfo > fit_info;

  void Reset() {
    org_chart = orig_org_chart;
    fitness_chart = orig_fitness_chart;
    prob_cache.clear();
    is_dominated.Resize(GetNumOrgs());
    is_dominated.Clear();
    is_active.Resize(GetNumOrgs());
    is_active.SetAll();
    is_discrim.Resize(GetNumCriteria());
    is_discrim.SetAll();
    org_info.resize(0);
    org_info.resize(GetNumOrgs());
    fit_info.resize(0);
    fit_info.resize(GetNumOrgs());
  }

  /// Helper function to convert a set of org fitnesses to ranks.
  /// Sets inactive orgs to zero, min fitness to 1, and max fitness to num orgs (others count up from 1)
  void CriterionToRanks(size_t fit_id) {
    pop_fit_t & fits = fitness_chart[fit_id]; // Get the set of fitness values.
    double min_fit = emp::FindMin(fits);

    // Build a map that we will use to convert fitnesses to rank categories
    std::map<double, size_t> fit_map;

    // Shift all fitnesses so min_fit is 1.0; set all non-active orgs to have a fitness of zero.
    for (size_t org_id = 0; org_id < fits.size(); org_id++) {
      if (is_active[org_id]) fits[org_id] = fits[org_id] - min_fit + 1.0;
      else fits[org_id] = 0.0;
      fit_map[fits[org_id]] = 0; // Make sure all fitnesses have an entry in the fitness map.
    }
    fit_map[0] = 0; // Make sure 0 is in the fitness map.

    // Loop through the fitness map assigning new fitness values.
    size_t new_fit = 0;
    for (auto & x : fit_map) x.second = new_fit++;

    // Make sure that the maximum fitness is always the number of organisms.
    double max_fit = emp::FindMax(fits);
    fit_map[max_fit] = fits.size();

    // Now, update all of the fitness values appropriately.
    for (double & fit : fits) fit = (double) fit_map[fit];
  }

  // Convert ALL criteria to be rank-based.
  void CriteriaToRanks() {
    for (size_t fit_id = 0; fit_id < fitness_chart.size(); fit_id++) {
      CriterionToRanks(fit_id);           // Convert this criterion so that it uses only ranks.
    }
  }

public:
  SelectionData()
    : org_chart(), fitness_chart(), prob_cache()
    , is_dominated(), is_active(), is_discrim()
    , org_info(), fit_info()
  { Reset(); }
  SelectionData(const std::string & filename, bool use_row_headings=true, bool use_col_headings=true)
    : SelectionData() {
    Load(filename, use_row_headings, use_col_headings);
  }
  SelectionData(const SelectionData &) = default;
  SelectionData(SelectionData &&) = default;
  ~SelectionData() { ; }

  SelectionData & operator=(const SelectionData &) = default;
  SelectionData & operator=(SelectionData &&) = default;

  size_t GetNumCriteria() const { return fitness_chart.size(); }
  size_t GetNumOrgs() const { return org_chart.size(); }

  const pop_fit_t & GetOrgData(size_t org_id) const { return org_chart[org_id]; }
  const pop_fit_t & GetFitData(size_t criterion_id=0) const { return fitness_chart[criterion_id]; }

  void SetVerbose(bool v=true) { verbose = v; }

  // Remove all fitnesses except for the one specified.
  bool SetFitnessID(size_t fit_id) {
    if (fit_id >= fitness_chart.size()) return false;  // Illegal fitness ID chosen.
    if (fitness_chart.size() == 1) return true;        // Only column, nothing to do.

    // If we are not working with the first fitness ID, copy to position 0.
    if (fit_id > 0) {
      for (size_t org_id = 0; org_id < org_chart.size(); org_id++) {
        org_chart[org_id][0] = org_chart[org_id][fit_id];
      }
      fitness_chart[0] = fitness_chart[fit_id];
    }

    // Eliminate all other traits.
    for (size_t org_id = 0; org_id < org_chart.size(); org_id++) org_chart[0].resize(1);
    fitness_chart.resize(1);

    // Since this alteration is being done by the user, mark the new versions as "original".
    orig_org_chart = org_chart;
    orig_fitness_chart = fitness_chart;

    return true;
  }

  /// Load a file with fitness data.
  /// * File is structed as a CSV using '#' for comments.
  /// * First row is column headings
  /// * Additional ROWS represent organisms
  /// * COLS represent selection criteria (e.g. fitness function results)
  void Load(const std::string & filename, bool use_row_headings=true, bool use_col_headings=true) {
    emp::File file(filename);                  // Load in file data.
    file.RemoveComments('#');                  // Trim off any comments beginning with a '#'
    file.RemoveEmpty();                        // Remove empty lines that used to have comments.
    if (use_col_headings) file.ExtractRow();   // Load in the column headers in the first row.
    if (use_row_headings) file.ExtractCol();   // Load in the row headers in the first row.
    file.RemoveWhitespace();                   // Remove all remaining spaces and tabs.
    org_chart = file.ToData<double>();         // Load fitness data for each organism from file.
    fitness_chart = emp::Transpose(org_chart); // Also organize data based on fitnesses.
    orig_org_chart = org_chart;
    orig_fitness_chart = fitness_chart;
    org_info.resize(org_chart.size());         // Track info for all organisms.
    fit_info.resize(fitness_chart.size());     // Track info for all criteria.
  }

  void PrintOrgs(std::ostream & os=std::cout) {
    const size_t num_orgs = GetNumOrgs();
    for (size_t org_id = 0; org_id < num_orgs; org_id++) {
      const emp::vector<double> & org = org_chart[org_id];
      for (double fit : org) {
        os << fit << " ";
      }
      os << " (prob = " << org_info[org_id].select_prob << ")";
      if (is_dominated[org_id]) os << "  DOMINATED";
      else if (is_active[org_id] == false) os << "  DUPLICATE";
      os << std::endl;
    }
  }

  void PrintCriteria(std::ostream & os=std::cout) {
    const size_t num_fits = GetNumCriteria();
    for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
      const emp::vector<double> & crit = fitness_chart[fit_id];
      for (double fit : crit) {
        os << fit << " ";
      }
      if (is_discrim[fit_id] == false) os << "  NON-DISCIMINATORY";
      os << std::endl;
    }
  }

  void PrintNewCriteria(std::ostream & os=std::cout) {
    const size_t num_fits = GetNumCriteria();
    for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
      if (is_discrim[fit_id] == false) continue;
      const emp::vector<double> & crit = fitness_chart[fit_id];
      for (size_t org_id = 0; org_id < crit.size(); org_id++) {
        if (is_active[org_id] == false) continue;
        os << crit[org_id] << " ";
      }
      os << std::endl;
    }
  }

  emp::vector<double> GetSelectProbs(std::ostream & os=std::cout, bool sort_output=false) {
    emp::vector<double> probs(GetNumOrgs());
    for (size_t i = 0; i < GetNumOrgs(); i++) {
      probs[i] = org_info[i].select_prob;
    }
    if (sort_output) std::sort(probs.rbegin(), probs.rend());
    return probs;
  }

  void PrintSelectProbs(std::ostream & os=std::cout, bool sort_output=false) {
    emp::vector<double> probs = GetSelectProbs(os, sort_output);
    for (size_t i = 0; i < probs.size(); i++) {
      if (i) os << ',';
      os << probs[i];
    }
    os << std::endl;
  }

  // Loop through all pairs of active organisms.  If any are dominated, remove them.
  // Return how much progress we made on reducing the number of organisms being considered.
  size_t AnalyzeLexicase_RemoveDominated() {
    const size_t num_orgs = GetNumOrgs();
    const size_t num_fits = GetNumCriteria();
    size_t progress = 0;

    for (size_t org1_id = 0; org1_id < num_orgs; org1_id++) {
      if (is_active[org1_id] == false) continue;     // This org has already been dominated.

      const emp::vector<double> & org1 = org_chart[org1_id];

      // Track anything that org1 dominates or duplicates
      for (size_t org2_id = org1_id+1; org2_id < num_orgs; org2_id++) {
        if (is_active[org2_id] == false) continue;     // This org has already been dominated.
        const emp::vector<double> & org2 = org_chart[org2_id];

        bool maybe_dom1 = true;
        bool maybe_dom2 = true;
        for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
          if (!is_discrim[fit_id]) continue;    // Ignore functions that have already been removed.
          if (org1[fit_id] < org2[fit_id]) {
            maybe_dom1 = false;
            if (maybe_dom2 == false) break;
          }
          if (org1[fit_id] > org2[fit_id]){
            maybe_dom2 = false;            
            if (maybe_dom1 == false) break;
          }
        }

        // Both TRUE                                 => DUPLICATE
        // Both FALSE                                => No dominance
        // maybe_dom1 == true && maybe_dom2 == false => ORG1 dominates        
        // maybe_dom1 == false && maybe_dom2 == true => ORG2 dominates        

        if (maybe_dom1 == true && maybe_dom2 == true) {
          // This is a duplicate, so mark and remove org2 from additional consideration.
          org_info[org1_id].dup_ids.push_back(org2_id);
          is_active[org2_id] = false;
          progress++;
        }

        else if (maybe_dom1 == true && maybe_dom2 == false) {
          // Org 1 dominates org 2.  Mark org2 as dominated and inactive.
          is_active[org2_id] = false;
          is_dominated[org2_id] = true;
          progress++;
        }

        else if (maybe_dom2 == true && maybe_dom1 == false) {
          // Org 2 dominates org 1.  Mark org1 as dominated and inactive.
          is_active[org1_id] = false;
          is_dominated[org1_id] = true;
          progress++;
        }
      }
    }

    return progress;
  }

  // Remove any criteria that are not discriminatory among viable organisms.
  size_t AnalyzeLexicase_RemoveNonDiscriminatory() {
    size_t progress = 0;

    // Convert the fitness chart to use ranks instead of input values.
    // All non-active organisms should have ranks of zero.
    CriteriaToRanks();

    // Any criterion where all fitness values are 0 or max is non-discriminatory.
    for (size_t fit_id = 0; fit_id < fitness_chart.size(); fit_id++) {
      if (is_discrim[fit_id] == false) continue; // Already non-discriminatory.

      bool discrim = false;
      double max_fit = (double) GetNumOrgs();
      for (size_t org_id = 0; org_id < GetNumOrgs(); org_id++) {
        if (!is_active[org_id]) continue;             // Only consider on active organisms.
        double fit = fitness_chart[fit_id][org_id];   // Get fitness for this organism.
        if (fit != 0 && fit != max_fit) {
          discrim = true;
          break;
        }
      }
      if (!discrim) {
        is_discrim[fit_id] = false;
        progress++;
      }
    }

    return progress;
  }

  // Remove any organisms that do not win (or tie for win) on ANY criteria.
  size_t AnalyzeLexicase_RemoveHopelessOrgs() {
    // If there are no fitness functions left, skip this step.
    if (is_discrim.None()) return 0;

    size_t progress = 0;

    double max_fit = (double) GetNumOrgs();

    for (size_t org_id = 0; org_id < GetNumOrgs(); org_id++) {
      if (is_active[org_id] == false) continue;     // This org has already been removed.
      bool can_dom = false;
      for (size_t fit_id = 0; fit_id < GetNumCriteria(); fit_id++) {
        if (is_discrim[fit_id] && fitness_chart[fit_id][org_id] == max_fit) {
          can_dom = true;
          break;
        }
      }
      if (can_dom == false) {
        is_active[org_id] = false;
        is_dominated[org_id] = true;
        progress++;
      }
    }

    return progress;
  }

  size_t AnalyzeLexicase_RemoveDuplicateCriteria() {
    size_t progress = 0;

    // Make sure criteria are in rank form for easy comparison.
    CriteriaToRanks();

    for (size_t fit_id1 = 0; fit_id1 < fitness_chart.size(); fit_id1++) {
      if (is_discrim[fit_id1] == false) continue; // This criterion has already been eliminated.

      for (size_t fit_id2 = fit_id1+1; fit_id2 < fitness_chart.size(); fit_id2++) {
        if (is_discrim[fit_id2] == false) continue; // This criterion has already been eliminated.

        // If we perfectly duplicate another criterion, mark it as a duplicate and deactivate it.
        if (fitness_chart[fit_id1] == fitness_chart[fit_id2]) {
          fit_info[fit_id1].dup_ids.push_back(fit_id2);
          is_discrim[fit_id2] = false;
          progress++;
        }
      }
    }

    return progress;
  }

  void AnalyzeLexicase(bool reset_orgs=true) {
    if (reset_orgs) Reset();

    if (verbose) {
      std::cout << "Starting AnalyzeLexicase.\n"
                << "Before: org count=" << is_active.CountOnes()
                << ";  criteria count=" << is_discrim.CountOnes()
                << std::endl;
    }

    size_t progress = 1;

    while (progress) {
      progress = 0;

      emp_assert(is_active.Any());

      // Compare all orgs to find direct domination.
      progress += AnalyzeLexicase_RemoveDominated();
      if (verbose) {
        std::cout << "After RemoveDominated, progress count = " << progress
                  << ";  active count = " << is_active.CountOnes()
                  << ";  criteria count = " << is_discrim.CountOnes()
                  << std::endl;
      }

      emp_assert(is_active.Any());

      // Remove criteria that cannot discriminate among orgs.
      progress += AnalyzeLexicase_RemoveNonDiscriminatory();
      if (verbose) {
        std::cout << "After RemoveNonDiscriminatory, progress count = " << progress
                  << ";  active count = " << is_active.CountOnes()
                  << ";  criteria count = " << is_discrim.CountOnes()
                  << std::endl;
      }

      emp_assert(is_active.Any());

      // Remove orgs that cannot win on any criteria.
      progress += AnalyzeLexicase_RemoveHopelessOrgs();
      if (verbose) {
        std::cout << "After RemoveHopelessOrgs, progress count = " << progress
                  << ";  active count = " << is_active.CountOnes()
                  << ";  criteria count = " << is_discrim.CountOnes()
                  << std::endl;
      }

      emp_assert(is_active.Any());

      // Remove duplicate criteria (that perform identically to others)
      progress += AnalyzeLexicase_RemoveDuplicateCriteria();
      if (verbose) {
        std::cout << "After RemoveDuplicateCriteria, progress count = " << progress
                  << ";  active count = " << is_active.CountOnes()
                  << ";  criteria count = " << is_discrim.CountOnes()
                  << std::endl;
      }

      emp_assert(is_active.Any());
    }

  }

  /// Calculate the remaining probabilities for a given starting prob and
  /// current orgs and criteria.
  const emp::vector<double> & CalcLexicaseProbs(const emp::BitVector & orgs, const emp::BitVector & fits) {
    emp_assert(orgs.Any());  // Must have at lease one organism still in the mix!

    // Look up this set of organisms in the cache and return if found.  Keep a referece to the probailties
    // so that it will be automatically updated in the cache for next time we need it.
    emp::vector<double> & out_probs = prob_cache[orgs];
    if (out_probs.size()) return out_probs;

    // We haven't cached out_probs, so calculate it now; inititalize all probs to zero.
    out_probs.resize(orgs.GetSize(), 0.0);

    // Track the total weight of all the criteria to determine the fraction associated with each.
    double total_fit_weight = 0.0;

    // Loop through all criteria that we can run next.
    emp::BitVector next_orgs = orgs;
    emp::BitVector next_fits = fits;
    for (int fit_id = fits.FindBit(); fit_id != -1; fit_id = fits.FindBit(fit_id+1)) {
      double weight = fit_info[(size_t) fit_id].GetWeight();
      next_fits.Set((size_t) fit_id, false);  // Turn off this criterion so it can't be run again.

      // Trim down to just the orgs that make it past this criterion.
      double best_fit = 0.0;
      for (int org_id = orgs.FindBit(); org_id != -1; org_id = orgs.FindBit(org_id+1)) {
        const double cur_fit = fitness_chart[(size_t) fit_id][(size_t)org_id];
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          next_orgs.Clear();
        }
        if (cur_fit == best_fit) {
          next_orgs.Set((size_t) org_id);
        }
      }

      // If this criterion made no progress, abandon it as non-discriminatory.
      if (next_orgs == orgs) continue;

      total_fit_weight += weight;

      // Recursively call on the next population.
      const auto & next_probs = CalcLexicaseProbs(next_orgs, next_fits);
      for (size_t org_id = 0; org_id < out_probs.size(); org_id++) {
        out_probs[org_id] += weight * next_probs[org_id];
      }
      next_fits.Set((size_t) fit_id, true);   // Turn back on this criterion for next loop.
    }

    // If there are no fitness functions left, all organisms have an equal chance.
    if (total_fit_weight == 0.0) {
      double total_org_weight = 0.0;
      for (int org_id = orgs.FindBit(); org_id != -1; org_id = orgs.FindBit(org_id+1)) {
        total_org_weight += org_info[org_id].GetWeight();
      }
      for (int org_id = orgs.FindBit(); org_id != -1; org_id = orgs.FindBit(org_id+1)) {
        out_probs[org_id] = ((double) org_info[org_id].GetWeight()) / total_org_weight;
      }
      return out_probs;
    }

    emp::Scale(out_probs, 1.0 / total_fit_weight);
    return out_probs;
  }

  // Calculate the probabilities of each organism being selected in lexicase...
  void CalcLexicaseProbs() {
    emp_assert(is_active.Any());

    // Setup all singleton populations in the cache.
    emp::BitVector base_orgs(org_info.size());
    std::vector<double> base_probs(org_info.size(), 0.0);
    for (size_t org_id = 0; org_id < org_info.size(); org_id++) {
      // Build a BitVector with just this organism.
      base_orgs.Set(org_id, true);
      base_probs[org_id] = 1.0;

      // Store it in the cache.
      prob_cache[base_orgs] = base_probs;

      // Clean up bit vector and probs for next round.
      base_orgs.Set(org_id, false);
      base_probs[org_id] = 0.0;
    }

    // CalcLexicaseProbs(1.0, is_active, is_discrim);
    auto results = CalcLexicaseProbs(is_active, is_discrim);
    for (size_t org_id = 0; org_id < org_info.size(); org_id++) {
      // Determine probability for each organism duplicated in this entry.
      const double each_prob = results[org_id] / org_info[org_id].GetWeight();

      // Assign probability to both this org and its duplicates.
      org_info[org_id].select_prob += each_prob;
      for (size_t dup_id : org_info[org_id].dup_ids) {
        org_info[dup_id].select_prob += each_prob;
      }
    }
  }

  void SampleOrgs(size_t num_keep, emp::Random & random) {
    emp_assert(num_keep <= GetNumOrgs());
    size_t num_remove = GetNumOrgs() - num_keep;
    if (num_remove == 0) return;
    auto remove_ids = emp::Choose(random, GetNumOrgs(), num_remove);
    for (size_t org_id : remove_ids) {
      is_active[org_id] = false;
    }
  }

  void SampleCriteria(size_t num_keep, emp::Random & random) {
    emp_assert(num_keep <= GetNumCriteria());
    size_t num_remove = GetNumCriteria() - num_keep;
    if (num_remove == 0) return;
    auto remove_ids = emp::Choose(random, GetNumCriteria(), num_remove);
    for (size_t fit_id : remove_ids) {
      is_discrim[fit_id] = false;
    }

  }

  emp::vector<double> CalcSubsampleLexicaseProbs(size_t orgs_used, size_t fits_used,
                                                 size_t num_tests, emp::Random & random) {
    emp::vector<double> total_probs(GetNumOrgs(), 0.0);
    emp::vector<double> cur_probs;
    for (size_t test_id = 0; test_id < num_tests; test_id++) {
      std::cout << "Running test #" << test_id << std::endl;
      Reset();
      SampleOrgs(orgs_used, random);
      SampleCriteria(fits_used, random);
      if (verbose) std::cout << "End-subsample, orgs=" << is_active << " ; fits=" << is_discrim << std::endl;
      emp_assert(is_active.Any());
      emp_assert(is_discrim.Any());
      AnalyzeLexicase(false);      // Run an analysis, but do not reset the population (to keep sample).
      emp_assert(is_active.Any());
      CalcLexicaseProbs();
      cur_probs = GetSelectProbs();
      for (size_t i = 0; i < GetNumOrgs(); i++) {
        if (verbose) std::cout << cur_probs[i] << " ";
        total_probs[i] += cur_probs[i];
      }
      if (verbose) std::cout << std::endl << std::endl;
    }

    // Convert the totals into an average and return the vector.
    for (double & x : total_probs) x /= (double) num_tests;
    return total_probs;
  }
};

#endif
