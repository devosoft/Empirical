//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Calculate probabilities for selection using Roulette Selection (easy!)


#include "SelectionData.hpp"
#include "emp/datastructs/IndexMap.hpp"

int main()
{
  SelectionData data("example.dat");
  std::cout << "Fitnesses:" << std::endl;
  data.PrintCriteria();

  const size_t num_criteria = data.GetNumCriteria();
  const size_t num_orgs = data.GetNumOrgs();

  if (num_criteria == 0) {
    std::cout << "Warning: No fitness data found." << std::endl;
  }
  if (num_criteria > 1) {
    std::cout << "Warning: predicting roulette with more than one criterion.  Using only first." << std::endl;
  }

  emp::vector< double > fit_data = data.GetFitData();
  emp::IndexMap fit_map(num_orgs);
  for (size_t i = 0; i < num_orgs; i++) {
    fit_map[i] = fit_data[i];
  }
  for (size_t i = 0; i < num_orgs; i++) {
    std::cout << fit_map.GetProb(i) << " ";
  }
  std::cout << std::endl;
}

