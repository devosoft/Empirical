// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/command_line.h"
#include "../../tools/unit_tests.h"

#undef NDEBUG
#define TDEBUG 1

#include "../../tools/assert.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert(true);
  EMP_TEST_VALUE(emp::assert_last_fail, "0");

  emp_assert(100);
  EMP_TEST_VALUE(emp::assert_last_fail, "0");

  emp_assert(23 < 24);
  EMP_TEST_VALUE(emp::assert_last_fail, "0");

  emp_assert((14 < 13)?0:1);
  EMP_TEST_VALUE(emp::assert_last_fail, "0");


  // Now here are some that should FAIL
  emp_assert(false);
  EMP_TEST_VALUE(emp::assert_last_fail, "1");
  EMP_TEST_VALUE(emp::assert_fail_info.filename, "assert.cc");
  EMP_TEST_VALUE(emp::assert_fail_info.line_num, "31");
  EMP_TEST_VALUE(emp::assert_fail_info.error, "false");

  // if (emp::assert_fail_info.filename != "assert.cc") std::cerr << "Failed case 6!" << std::endl;
  // if (emp::assert_fail_info.line_num != __LINE__ - 3) std::cerr << "Failed case 7!" << std::endl;
  // if (emp::assert_fail_info.error != "false") std::cerr << "Failed case 8!" << std::endl;

  return 0;
}
