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

#include "../../source/base/vector.h"
#include "../../source/tools/BitVector.h"
#include "../../source/tools/File.h"
#include "../../source/tools/vector_utils.h"

class SelectionData {
private:
  using pop_fit_t = emp::vector<double>;  ///< Type for individual fitnesses for a single function.
  emp::vector< pop_fit_t > org_chart;     ///< Chart of all fitnesses for each organism.
  emp::vector< pop_fit_t > fitness_chart; ///< Chart of all fitnesses for each function.

  emp::BitVector is_dominated;            ///< Is org guaranteed to have a chance?
  emp::BitVector is_active;               ///< Is org non-dominated AND not a duplicate.

  emp::BitVector is_discrim;              ///< Is criterium discrimanatory.

  struct OrgInfo {
    emp::vector< size_t > dup_ids;
  };

  struct CriterionInfo {
    emp::vector< size_t > dup_ids;
  };

  emp::vector< OrgInfo > org_info;

  void Reset() {
    is_dominated.Resize(GetNumOrgs());
    is_active.Resize(GetNumOrgs());
    is_discrim.Resize(GetNumCriteria());
    is_dominated.Clear();
    is_active.SetAll();
    is_discrim.SetAll();
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
  SelectionData() { Reset(); }
  SelectionData(const std::string & filename) {
    Reset();
    Load(filename);
  }
  ~SelectionData() { ; }

  size_t GetNumCriteria() const { return fitness_chart.size(); }
  size_t GetNumOrgs() const { return org_chart.size(); }

  const pop_fit_t & GetOrgData(size_t org_id) const { return org_chart[org_id]; }
  const pop_fit_t & GetFitData(size_t criterion_id=0) const { return fitness_chart[criterion_id]; }

  /// Load a file with fitness data.
  /// * File is structed as a CSV using '#' for comments.
  /// * First row is column headings
  /// * Additional ROWS represent organisms
  /// * COLS represent selection criteria (e.g. fitness function results)
  void Load(const std::string & filename) {
    emp::File file(filename);              // Load in file data.
    file.RemoveComments('#');              // Trim off any comments beginning with a '#'
    file.RemoveEmpty();                    // Remove any line that used to have comments and are now empty.
    auto headers = file.ExtractRow();      // Load in the column headers in the first row.
    file.RemoveWhitespace();               // Remove all remaining spaces and tabs.
    org_chart = file.ToData<double>();     // Load in fitness data for each organism from file.
    fitness_chart = emp::Transpose(org_chart); // Organize data based on fitnesses rather than organisms.
    org_info.resize(org_chart.size());     // Track info for all organisms.
  }

  void PrintOrgs(std::ostream & os=std::cout) {
    const size_t num_orgs = GetNumOrgs();
    const size_t num_fits = GetNumCriteria();
    for (size_t org_id = 0; org_id < num_orgs; org_id++) {
      const emp::vector<double> & org = org_chart[org_id];
      for (double fit : org) {
        os << fit << " ";
      }
      if (is_dominated[org_id]) os << "  DOMINATED";
      else if (is_active[org_id] == false) os << "  DUPLICATE";
      os << std::endl;
    }
  }

  void PrintCriteria(std::ostream & os=std::cout) {
    const size_t num_orgs = GetNumOrgs();
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

  // Loop through all pairs of active organisms.  If any are dominated, remove them.
  // Return how much progress we made on reducing the number of organisms being considered.
  size_t AnalyzeDominance_CompareOrgs() {
    const size_t num_orgs = GetNumOrgs();
    const size_t num_fits = GetNumCriteria();
    size_t progress = 0;

    for (size_t org1_id = 0; org1_id < num_orgs; org1_id++) {
      if (is_active[org1_id] == false) continue;     // This org has already been dominated.

      const emp::vector<double> & org1 = org_chart[org1_id];

      // Track anything that org1 dominates or duplicates
      for (size_t org2_id = org1_id+1; org2_id < num_orgs; org2_id++) {
        const emp::vector<double> & org2 = org_chart[org2_id];

        bool maybe_dom1 = true;
        bool maybe_dom2 = true;
        for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
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
  size_t AnalyzeDominance_RemoveNonDiscriminatory() {
    size_t progress = 0;

    // Convert the fitness chart to use ranks instead of input values.
    // All non-active organisms should have ranks of zero.
    CriteriaToRanks();

    // Any criterion where all fitness values are 0 or max is non-discriminatory.
    for (size_t fit_id = 0; fit_id < fitness_chart.size(); fit_id++) {
      bool discrim = false;
      size_t max_fit = GetNumOrgs();
      for (double fit : fitness_chart[fit_id]) {
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

  void AnalyzeDominance() {
    Reset();

    const size_t num_orgs = GetNumOrgs();
    const size_t num_fits = GetNumCriteria();

    size_t progress = 0;

    progress += AnalyzeDominance_CompareOrgs();
    progress += AnalyzeDominance_RemoveNonDiscriminatory();
  }
};

#endif
