//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class maintains all of the fitness data for a population of organisms.


#ifndef SELECTION_DATA_H
#define SELECTION_DATA_H

#include <iostream>
#include <string>

#include "../../source/base/vector.h"
#include "../../source/tools/File.h"

class SelectionData {
private:
  using pop_fit_t = emp::vector<double>;  ///< Type for individual fitnesses for a single function.
  emp::vector< pop_fit_t > fitness_chart; ///< Chart of all fitnesses for each function.

public:
  SelectionData() = default;
  ~SelectionData() { ; }

  void Load(const std::string & filename) {
    emp::File file(filename);              // Load in file data.
    file.RemoveComments('#');              // Trim off any comments beginning with a '#'
    file.RemoveWhitespace();               // Remove spaces and tabs (file should be CSV.)
    fitness_chart = file.ToData<double>(); // Load in fitness data from file.
  }

  void PrintFitnesses(ostream & os=std::cout) {
    for (const pop_fit_t & fit_row : fitness_chart) {
      for (double fit : fit_row) {
        os << fit << " ";
      }
      os << std::endl;
    }
  }
};

#endif
