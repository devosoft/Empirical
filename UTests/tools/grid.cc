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

  emp::Grid::Board<bool, int, bool> board(layout);
}
