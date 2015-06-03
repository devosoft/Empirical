#include <iostream>
#include <sstream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/DynamicStringSet.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::DynamicStringSet test_set;

  test_set.Append("Line Zero");  // Test regular append
  test_set << "Line One";        // Test stream append

  // Test append-to-stream
  std::stringstream ss;
  ss << test_set;
  emp_assert(ss.str() == "Line ZeroLine One");

  // Test direct conversion to string.
  emp_assert(test_set.str() == "Line ZeroLine One");

  // Test appending functions.
  test_set.Append( [](){ return std::string("Line Two"); } );
  test_set.Append( [](){ return "Line Three"; } );

  // Test appending functions with variable output
  int line_no = 20;
  test_set.Append( [&line_no](){ return std::string("Line ") + std::to_string(line_no); } );

  emp_assert(test_set[4] == "Line 20");
  line_no = 4;
  emp_assert(test_set[4] == "Line 4");

  // Make sure we can change an existing line.
  test_set.Set(0, "Line 0");
  emp_assert(test_set[0] == "Line 0");

  // If verbose, print the current status of lines.
  if (verbose) {
    for (int i = 0; i < test_set.GetSize(); i++) {
      std::cout << test_set[i] << std::endl;
    }
  }

  // Make sure all lines are what we expect.
  emp_assert(test_set[0] == "Line 0");
  emp_assert(test_set[1] == "Line One");
  emp_assert(test_set[2] == "Line Two");
  emp_assert(test_set[3] == "Line Three");
  emp_assert(test_set[4] == "Line 4");
}
