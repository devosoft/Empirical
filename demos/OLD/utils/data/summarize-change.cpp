/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief This file takes in a column name and one or more CSV files and summarizes that value's
 *  change over time (with the assumption that each row is another time point)
 */

#include <fstream>
#include <iostream>
#include <string>

#include "../../../include/emp/base/assert.hpp"
#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/tools/String.hpp"

using data_t = emp::vector< emp::vector<double> >;

void ProcessFile(const std::string & filename, size_t col_id, data_t & data, std::ostream & os=std::cout) {
  std::cerr << "Processing file: " << filename << std::endl;
  std::ifstream is(filename);

  // Make sure the file opened correctly.
  if (is.fail()) {
    std::cerr << "Unable to open data file '" << filename << "'. Ignoring." << std::endl;
    return;
  }

  data.emplace_back(0);
  auto & cur_data = data.back();

  // Loop through the file!
  emp::String cur_line;
  emp::vector<emp::String> cols;        // Create a vector to store cols in.
  std::getline(is, cur_line);           // Skip the first line (headers)

  while (!is.eof()) {
    std::getline(is, cur_line);         // Collect the next line to process
    cur_line.RemoveWhitespace();        // Strip out all whitespace for simplicity
    if (cur_line.size() == 0 || cur_line[0] == '#') continue;   // Skip empty lines or those beginning with a comment.
    cols = cur_line.Slice(",");
    if (cols.size() <= col_id) {
      std::cerr << "Skipping line - not enough columns." << std::endl;
      continue;
    }
    cur_data.push_back(cols[col_id].AsDouble());
  }
}

void PrintData(const data_t & data, bool print_all) {
  if (data.size() == 0) {
    std::cerr << "ERROR: No data found.  Aborting." << std::endl;
    exit(1);
  }

  // Count the min and max number of rows in any column.
  size_t max_rows = 0;
  size_t min_rows = data[0].size();
  for (const auto & col : data) {
    if (col.size() < min_rows) min_rows = col.size();
    if (col.size() > max_rows) max_rows = col.size();
  }

  // If we are printing everything, do so.
  if (print_all) {
    for (size_t row_id = 0; row_id < max_rows; ++row_id) {
      for (size_t col_id = 0; col_id < data.size(); ++col_id) {
        if (col_id) std::cout << ",";
        if (data[col_id].size() > row_id) { std::cout << data[col_id][row_id]; }
      }
      std::cout << '\n';
    }
  }

  // Otherwise just print summary info.
  else {
    if (min_rows != max_rows) {
      std::cerr << "ERROR: All files must have same amount of data to summarize." << std::endl;
      exit(1);
    }

    std::cout << "#min,mean,max\n";
    for (size_t row_id = 0; row_id < max_rows; ++row_id) {
      double min_val = data[0][row_id];
      double max_val = data[0][row_id];
      double total = 0.0;

      for (size_t col_id = 0; col_id < data.size(); ++col_id) {
        if (data[col_id][row_id] < min_val) min_val = data[col_id][row_id];
        if (data[col_id][row_id] > max_val) max_val = data[col_id][row_id];
        total += data[col_id][row_id];
      }

      // And print the summary:
      std::cout << min_val << ',' << (total/data.size()) << ',' << max_val << '\n';
    }
  }
}

int main(int argc, char* argv[])
{
  emp::vector<emp::String> args = emp::cl::ArgsToStrings(argc, argv);
  const bool print_all = emp::cl::UseArg(args, "-a");
  const bool verbose = emp::cl::UseArg(args, "-v");

  if (args.size() < 3) {
    std::cerr << "Must provide one column number (starting from 0) and at least one filename!\n"
      << "Format: " << args[0] << " [options] column_number filename [filnames...]\n"
      << "Options:\n"
      << "  -a Print ALL data, not summary.\n"
      << "  -v Use verbose output.\n"
      << std::endl;
    exit(1);
  }

  emp::String column(args[1]);
  if (!column.IsNumber()) {
    std::cerr << "First argument must be a column number." << std::endl;
    exit(1);
  }

  size_t col_id = column.AsULL();
  data_t data;

  // Process each file that was passed in.
  for (size_t arg_id = 2; arg_id < args.size(); arg_id++) {
    ProcessFile(args[arg_id], col_id, data);
  }

  if (verbose) {
    std::cout << "Cols: " << data.size() << std::endl;
    std::cout << "Rows: " << data[0].size() << std::endl;
    std::cout << "Row 0, Col 0: " << data[0][0] << std::endl;
    std::cout << "Row 0, Col 1: " << data[1][0] << std::endl;
    std::cout << "Row 1, Col 0: " << data[0][1] << std::endl;
    std::cout << "Row 1, Col 1: " << data[1][1] << std::endl;
  }

  // Output the resulting data
  PrintData(data, print_all);


  return 0;
}
