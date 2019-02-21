//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Calculate probabilities for selection using Roulette Selection (easy!)

#include "config/command_line.h"

#include "SelectionData.h"

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  std::string filename = "example.dat";
  if (args.size() > 1) filename = args[1];
  SelectionData data(filename);

  if (data.GetNumCriteria() == 0) {
    std::cout << "Warning: No criteria data found." << std::endl;
  }

  data.AnalyzeLexicase();
  data.CalcLexicaseProbs();

  std::cout << "By organism (" << data.GetNumOrgs() << "):" << std::endl;
  data.PrintOrgs();
  std::cout << std::endl;
  std::cout << "By criterion (" << data.GetNumCriteria() << "):" << std::endl;
  data.PrintCriteria();

  std::cout << std::endl;
  data.PrintNewCriteria();
  
  std::cout << std::endl;
  data.PrintSelectProbs();

  // emp::vector< double > fit_data = data.GetFitData(0);
  // emp::IndexMap fit_map(num_orgs);
  // for (size_t i = 0; i < num_orgs; i++) {
  //   fit_map[i] = fit_data[i];
  // }
  // for (size_t i = 0; i < num_orgs; i++) {
  //   std::cout << fit_map.GetProb(i) << " ";
  // }
  // std::cout << std::endl;
}

