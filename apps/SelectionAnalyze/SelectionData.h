//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class maintains all of the fitness data for a population of organisms.


#ifndef SELECTION_DATA_H
#define SELECTION_DATA_H

#include <iostream>
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

  emp::BitVector is_dominated;            ///< Is each org guaranteed to have a chance?
  emp::BitVector is_active;               ///< Non-dominated AND not a duplicate.

  struct OrgInfo {
    emp::vector< size_t > dup_ids;
  };

  emp::vector< OrgInfo > org_info;

  void Reset() {
    is_dominated.Resize(GetNumOrgs());
    is_active.Resize(GetNumOrgs());
    is_dominated.Clear();
    is_active.SetAll();
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

  void PrintFitnesses(std::ostream & os=std::cout) {
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

//#define NOTE(X) std::cout << X << std::endl

  void AnalyzeDominance() {
    Reset();

    const size_t num_orgs = GetNumOrgs();
    const size_t num_fits = GetNumCriteria();

    for (size_t org1_id = 0; org1_id < num_orgs; org1_id++) {
//      NOTE("ORG1 = " << org1_id);
      if (is_active[org1_id] == false) {
//        NOTE("..SKIPPING -- org already deactivated");
        continue;                  // This org has already been dominated.
      }
      const emp::vector<double> & org1 = org_chart[org1_id];

      // Track anything that org1 dominates or duplicates
      for (size_t org2_id = org1_id+1; org2_id < num_orgs; org2_id++) {
//        NOTE("  ORG2 = " << org2_id);
        const emp::vector<double> & org2 = org_chart[org2_id];

        bool maybe_dom1 = true;
        bool maybe_dom2 = true;
        for (size_t fit_id = 0; fit_id < num_fits; fit_id++) {
          if (org1[fit_id] < org2[fit_id]) {
//            NOTE("    org1 < org2 on fit ID " << fit_id);
            maybe_dom1 = false;
            if (maybe_dom2 == false) break;
          }
          if (org1[fit_id] > org2[fit_id]){
//            NOTE("    org1 > org2 on fit ID " << fit_id);
            maybe_dom2 = false;            
            if (maybe_dom1 == false) break;
          }
        }

//        NOTE("    result = " << maybe_dom1 << " " << maybe_dom2);

        // Both TRUE                                 => DUPLICATE
        // Both FALSE                                => No dominance
        // maybe_dom1 == true && maybe_dom2 == false => ORG1 dominates        
        // maybe_dom1 == false && maybe_dom2 == true => ORG2 dominates        

        if (maybe_dom1 == true && maybe_dom2 == true) {
//          NOTE("    RESULT: Marking org2 as a duplicate!");
          // This is a duplicate, so mark and remove org2 from additional consideration.
          org_info[org1_id].dup_ids.push_back(org2_id);
          is_active[org2_id] = false;
        }

        else if (maybe_dom1 == true && maybe_dom2 == false) {
//          NOTE("    RESULT: Marking org2 as dominated!");
          // Org 1 dominates org 2.  Mark org2 as dominated and inactive.
          is_active[org2_id] = false;
          is_dominated[org2_id] = true;
        }

        else if (maybe_dom2 == true && maybe_dom1 == false) {
//          NOTE("    RESULT: Marking org1 as dominated!");
          // Org 2 dominates org 1.  Mark org1 as dominated and inactive.
          is_active[org1_id] = false;
          is_dominated[org1_id] = true;
        }
      }
    }

  }
};

#endif
