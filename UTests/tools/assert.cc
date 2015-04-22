#include <iostream>

#undef NDEBUG
#define TDEBUG 1

#include "../../tools/assert.h"

int main()
{
  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert(true);
  if (emp::assert_last_fail) std::cerr << "Failed case 1!" << std::endl;

  emp_assert(100);
  if (emp::assert_last_fail) std::cerr << "Failed case 2!" << std::endl;

  emp_assert(23 < 24);
  if (emp::assert_last_fail) std::cerr << "Failed case 3!" << std::endl;

  emp_assert((14 < 13)?0:1);
  if (emp::assert_last_fail) std::cerr << "Failed case 4!" << std::endl;


  // Now here are some that should FAIL
  emp_assert(false);
  if (!emp::assert_last_fail) std::cerr << "Failed case 5!" << std::endl;
  if (emp::assert_fail_info.filename != "assert.cc") std::cerr << "Failed case 6!" << std::endl;
  if (emp::assert_fail_info.line_num != __LINE__ - 3) std::cerr << "Failed case 7!" << std::endl;
  if (emp::assert_fail_info.error != "false") std::cerr << "Failed case 8!" << std::endl;

  return 0;
}
