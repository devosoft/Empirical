/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file reorder_cols.cpp
 *  @brief This utility loads in a space-separated file and reorders the columns.
 *
 *  Identify a file and a set of columns (starting with column 1).  Load the
 *  file and output the columns listed in the order given.
 *
 *    reorder_cols info.dat 5 2 7 3
 *
 *  ...will sent (to standard out) a file consisting of columns 5, 2, 7, and 3,
 *  ignoring all of the others.
 */

#include <iostream>
#include <string>

#include "../../../include/emp/config/command_line.hpp"
#include "../../../include/emp/io/File.hpp"
#include "../../../include/emp/tools/string_utils.hpp"

int main(int argc, char * argv[]) {
  auto args = emp::cl::args_to_strings(argc, argv);

  emp::File file(args[1]);
  file.RemoveEmpty();

  size_t max_col = 0;
  int num_cols = args.size() - 2;
  emp::vector<int> col_ids(num_cols);
  for (size_t i = 2; i < args.size(); i++) {
    col_ids[i-2] = std::stoi(args[i]);
    if (col_ids[i-2] > max_col) max_col = col_ids[i-2];
  }

  emp::vector< emp::vector<std::string> > cols(max_col);
  for (size_t i = 0; i < max_col; i++) {
    cols[i] = file.ExtractCol(' ');
  }

  for (size_t line_id = 0; line_id < cols[0].size(); line_id++) {
    for (size_t col_pos = 0; col_pos < num_cols; col_pos++) {
      // std::cout << '[' << line_id << ':' << col_pos << ']';
      // std::cout.flush();
      if (col_pos) std::cout << ' ';
      std::cout << cols[ col_ids[col_pos]-1 ][line_id];
    }
    std::cout << std::endl;
  }
}
