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
#include "../../source/tools/File.h"
#include "../../source/tools/vector_utils.h"

class SelectionData {
private:
  using pop_fit_t = emp::vector<double>;  ///< Type for individual fitnesses for a single function.
  emp::vector< pop_fit_t > org_chart;     ///< Chart of all fitnesses for each organism.
  emp::vector< pop_fit_t > fitness_chart; ///< Chart of all fitnesses for each function.

public:
  SelectionData() = default;
  SelectionData(const std::string & filename) { Load(filename); }
  ~SelectionData() { ; }

  size_t GetNumCriteria() const { return fitness_chart.size(); }
  size_t GetNumOrgs() const { return fitness_chart.size() ? fitness_chart[0].size() : 0; }

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
  }

  void PrintFitnesses(std::ostream & os=std::cout) {
    for (const pop_fit_t & fit_row : fitness_chart) {
      for (double fit : fit_row) {
        os << fit << " ";
      }
      os << std::endl;
    }
  }
};

#endif
