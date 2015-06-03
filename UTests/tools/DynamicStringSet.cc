#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/DynamicStringSet.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::DynamicStringSet test_set;

  test_set.Append("Line Zero");
  test_set.Append("Line One");
  test_set.Append( [](){ return std::string("Line Two"); } );
  test_set.Append( [](){ return "Line Three"; } );

  int line_no = 20;
  test_set.Append( [&line_no](){ return std::string("Line ") + std::to_string(line_no); } );
  line_no = 4;

  test_set.Set(0, "Line 0");

  if (verbose) {
    for (int i = 0; i < test_set.GetSize(); i++) {
      std::cout << test_set[i] << std::endl;
    }
  }

  emp_assert(test_set[0] == "Line 0");
  emp_assert(test_set[1] == "Line One");
  emp_assert(test_set[2] == "Line Two");
  emp_assert(test_set[3] == "Line Three");
  emp_assert(test_set[4] == "Line 4");
}
