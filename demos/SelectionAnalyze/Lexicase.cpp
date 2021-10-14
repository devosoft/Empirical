/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file Lexicase.cpp
 *  @brief Calculate probabilities for selection using Lexicase Selection.
 */

#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

#include "SelectionData.hpp"

int main(int argc, char* argv[])
{
  auto specs = emp::ArgManager::make_builtin_specs();
  specs["no_row_headings"] = emp::ArgSpec(0, "Turn off headings on each row.");
  specs["no_col_headings"] = emp::ArgSpec(0, "Turn off headings on each column.");
  specs["no_headings"] = emp::ArgSpec(0, "Turn off all headings.");
  specs["sample"] = emp::ArgSpec(3, "Sample reduced orgs & criteria.  Args: [num_orgs] [num_critera] [num_trials]");
  specs["sort"] = emp::ArgSpec(0, "Sort the output data (rather than keeping org position)");
  specs["verbose"] = emp::ArgSpec(0, "Print out lots of extra details about process.");
  specs["roulette"] = emp::ArgSpec(0, "Determine probabilities with roulette selection.");
  specs["elite"] = emp::ArgSpec(0, "Determine probabilities with elite selection");
  specs["fit_id"] = emp::ArgSpec(1, "Which column should be used for fitness (default = 0)");
  // NOTE: Tournament is just elite, but sample first.

  emp::ArgManager am(argc, argv, specs);
  if (am.ProcessBuiltin() == false) exit(0);

  bool use_row_headings = !am.UseArg("no_row_headings");
  bool use_col_headings = !am.UseArg("no_col_headings");
  if (am.UseArg("no_headings")) {
    use_row_headings = use_col_headings = false;
  }
  auto sample = am.UseArg("sample");
  bool sort_output = (bool) am.UseArg("sort");
  bool verbose = (bool) am.UseArg("verbose");

  bool do_roulette = (bool) am.UseArg("roulette");
  bool do_elite = (bool) am.UseArg("elite");

  size_t fit_id = 0;
  auto fit_id_info = am.UseArg("fit_id");
  if (fit_id_info->size() == 0) {
    std::cout << "Error: Fit ID flag has no arguments!" << std::endl;
    exit(1);
  }
  if (fit_id_info) fit_id = emp::from_string<size_t>((*fit_id_info)[0]);

  std::string command = argv[0];
  std::vector<std::string> filenames = *am.UseArg("_positional");

  if (filenames.size() < 1) {
    std::cerr << "Format: " << command << " [input file] {output file} {flags}." << std::endl;
    exit(1);
  }

  SelectionData data(filenames[0], use_row_headings, use_col_headings);
  if (verbose) data.SetVerbose();
  if (do_elite) {
    std::cout << "Using ELITE selection.  Fitness function id = " << fit_id << "." << std::endl;
    data.SetFitnessID(fit_id);
  }

  if (data.GetNumCriteria() == 0) {
    std::cout << "Warning: No criteria data found." << std::endl;
  }

  std::ofstream out_file;
  if (filenames.size() > 1) {
    std::cout << "Opening output file: " << filenames[1] << std::endl;
    out_file.open(filenames[1]);
  }
  std::ostream & out_stream = (filenames.size() > 1) ? out_file : std::cout;

  if (!sample) {
    data.AnalyzeLexicase();
    data.CalcLexicaseProbs();

    // std::cout << "By organism (" << data.GetNumOrgs() << "):" << std::endl;
    // data.PrintOrgs();
    // std::cout << std::endl;
    // std::cout << "By criterion (" << data.GetNumCriteria() << "):" << std::endl;
    // data.PrintCriteria();

    // std::cout << std::endl;
    // data.PrintNewCriteria();

    data.PrintSelectProbs(out_stream, sort_output);
  }

  else {
    size_t sample_pop = emp::from_string<size_t>((*sample)[0]);
    size_t sample_fits = emp::from_string<size_t>((*sample)[1]);
    size_t num_trials = emp::from_string<size_t>((*sample)[2]);

    emp::Random random;
    auto result = data.CalcSubsampleLexicaseProbs(sample_pop, sample_fits, num_trials, random);
    double total = 0.0;
    for (size_t i = 0; i < result.size(); i++) {
      if (i) out_stream << ',';
      out_stream << result[i];
      total += result[i];
    }
    out_stream << std::endl;
    std::cout << "Total prob = " << total << std::endl;
  }
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
