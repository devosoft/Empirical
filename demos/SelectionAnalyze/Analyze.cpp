//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Analyze probabilities for selection using various selection techniques.

#include "emp/config/command_line.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/math/Random.hpp"

#include "SelectionData.hpp"

int main(int argc, char* argv[])
{
  bool skip_row_headings = false;
  bool skip_col_headings = false;
  bool sort_output = false;
  bool verbose = false;

  emp::ArgManager args(argc, argv);
  args.SetOperands(1, "input_file", "output_file");  // One required operand (position arg), two named.
  args.AddFlag("--no_row_headings", "Turn off headings on each row.", skip_row_headings);
  args.AddFlag("--no_col_headings", "Turn off headings on each column.", skip_col_headings);
  args.AddFlag("--no_headings", "Turn off all headings.",
    [&skip_row_headings,&skip_col_headings](){ skip_row_headings=true; skip_col_headings=true; });
  args.AddFlag("--sort", "Sort the output data (rather than keeping org position)", sort_output);
  args.AddFlag("--verbose|-v", "Print out lots of extra details about process.", verbose);
  args.AddOption("--sample", 3, "Sample reduced orgs & criteria.", "num_orgs", "num_critera", "num_trials");

  if (args.Process() == false) exit(0);

  SelectionData data(args.Get("input_file"), !skip_row_headings, !skip_col_headings);
  if (verbose) data.SetVerbose();

  if (data.GetNumCriteria() == 0) {
    std::cout << "Warning: No criteria data found." << std::endl;
  }

  std::ofstream out_file;
  if (args.Get("output_file").size()) {
    std::cout << "Opening output file: " << args.Get("output_file") << std::endl;
    out_file.open(args.Get("output_file"));
  }
  std::ostream & out_stream = (args.Get("output_file").size()) ? out_file : std::cout;

  if (!args.HasOption("sample")) {
    data.AnalyzeLexicase();
    data.CalcLexicaseProbs();

    std::cout << "By organism (" << data.GetNumOrgs() << "):" << std::endl;
    data.PrintOrgs();
    std::cout << std::endl;
    std::cout << "By criterion (" << data.GetNumCriteria() << "):" << std::endl;
    data.PrintCriteria();

    std::cout << std::endl;
    data.PrintNewCriteria();

    data.PrintSelectProbs(out_stream, sort_output);
  }

  else {
    size_t sample_pop = args.GetOptionArg<size_t>("sample", "num_orgs");
    size_t sample_fits = args.GetOptionArg<size_t>("sample", "num_criteria");
    size_t num_trials = args.GetOptionArg<size_t>("sample", "num_trials");

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

}

