//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class maintains all of the fitness data for a population of organisms.


#ifndef SELECTION_DATA_H
#define SELECTION_DATA_H

#include "../../source/base/vector.h"

class SelectionData {
private:
  using pop_fitness_t = emp::vector<double>;
  emp::vector< pop_fitness_t > fitness_chart;

public:
  SelectionData() = default;
  ~SelectionData() { ; }
};

#endif
