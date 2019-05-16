//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Calculate probabilities for selection using Roulette Selection (easy!)

#include "config/command_line.h"
#include "config/ArgManager.h"

#include "SelectionData.h"

int main(int argc, char* argv[])
{
  auto specs = emp::ArgManager::make_builtin_specs();
  specs["no_row_headings"] = emp::ArgSpec(0, "Turn off headings on each row.");
  specs["no_col_headings"] = emp::ArgSpec(0, "Turn off headings on each column.");
  specs["no_headings"] = emp::ArgSpec(0, "Turn off all headings.");

  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  emp::ArgManager am(argc, argv, specs);
  am.PrintDiagnostic(std::cout);

  bool use_row_headings = !am.UseArg("no_row_headings");
  bool use_col_headings = !am.UseArg("no_col_headings");
  if (am.UseArg("no_headings")) {
    use_row_headings = use_col_headings = false;
  }

  std::string command = argv[0];
  std::vector<std::string> filenames = *am.UseArg("_positional");

  if (filenames.size() <= 1) {
    std::cerr << "Format: " << command << " [input file] {output file} {flags}." << std::endl;
    exit(1);
  }

  SelectionData data(filenames[1], use_row_headings, use_col_headings);

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

