// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/grid.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  if (verbose) {
    std::cout << "Testing grids." << std::endl;
  }

  emp::Grid::Layout layout(10,10);

  emp_assert(layout.GetNumRegions() == 0);
  layout.AddRegion({0,1,2,3,4,5,6,7,8});

  emp_assert(layout.GetNumRegions() == 1);

  emp::Grid::Board<void, void, void> board1(layout);
  emp::Grid::Board<bool, bool, bool> board2(layout);
  emp::Grid::Board<int, int, int> board3(layout);
  emp::Grid::Board<int, void, void> board4(layout);
  emp::Grid::Board<void, int, void> board5(layout);
  emp::Grid::Board<void, void, int> board6(layout);

  std::cout << sizeof(board1) << std::endl;
  std::cout << sizeof(board2) << std::endl;
  std::cout << sizeof(board3) << std::endl;
  std::cout << sizeof(board4) << std::endl;
  std::cout << sizeof(board5) << std::endl;
  std::cout << sizeof(board6) << std::endl;
}
