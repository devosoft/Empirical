/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2020
 *
 *  @file summarize.cpp
 *  @brief This file takes in one or more CSV files with values and, for each, calculates the mnimium,
 *  maximum, and average values found in each column.
 */

#include <fstream>
#include <iostream>
#include <string>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

void ProcessFile(const std::string & filename, std::ostream & os=std::cout) {
  os << "Processing file: " << filename << std::endl;
  std::ifstream is(filename);

  // Make sure the file opened correctly.
  if (is.fail()) {
    std::cerr << "Unable to open config file '" << filename << "'. Ignoring." << std::endl;
    return;
  }

  // Setup vectors to hold the data we need.
  emp::vector<size_t> col_size;
  emp::vector<double> col_min;
  emp::vector<double> col_max;
  emp::vector<double> col_total;

  // Loop through the file!
  std::string cur_line;
  emp::vector<std::string> cols;        // Create a vector to store cols in.
  while (!is.eof()) {
    std::getline(is, cur_line);         // Collect the next line to process
    emp::remove_whitespace(cur_line);   // Strip out all whitespace for simplicity
    if (cur_line[0] == '#') continue;   // Skip lines beginning with a comment.
    cols.resize(0);                     // Clear our vector for columns.
    emp::slice(cur_line, cols, ',');    // Split up this line by columns and put in cols.

    // Convert to doubles
    emp::vector<double> vals = emp::from_strings<double>(cols);

    // Make sure we are storing enough columns.
    size_t num_cols = cols.size();
    if (col_size.size() < cols.size()) {
      size_t old_size = col_size.size();
      col_size.resize(cols.size(), 0);
      col_min.resize(cols.size(), 0);
      col_max.resize(cols.size(), 0);
      col_total.resize(cols.size(), 0.0);

      // Use actual data to initialize min and max.
      col_min = vals;
      col_max = vals;
    }

    // Update all counts.
    for (size_t col_id = 0; col_id < num_cols; col_id++) {
      col_size[col_id]++;
      if (vals[col_id] < col_min[col_id]) col_min[col_id] = vals[col_id];
      if (vals[col_id] > col_max[col_id]) col_max[col_id] = vals[col_id];
      col_total[col_id] += vals[col_id];
    }
  }

  // Final processing.
  emp::vector<double> col_ave(col_total);
  for (size_t i = 0; i < col_ave.size(); i++) {
    col_ave[i] /= (double) col_size[i];
  }

  // Output the results.
  os << "Min: " << col_min << std::endl;
  os << "Max: " << col_max << std::endl;
  os << "Ave: " << col_ave << std::endl;
  os << "Tot: " << col_total << std::endl;
}

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  if (args.size() < 2) {
    std::cerr << "Must provide at least one filename!" << std::endl;
    exit(1);
  }

  for (size_t arg_id = 1; arg_id < args.size(); arg_id++) {
    ProcessFile(args[arg_id]);
  }

  return 0;
}
