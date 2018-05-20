//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the base/ folder.

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#define CATCH_CONFIG_MAIN
#undef NDEBUG
#define TDEBUG 1

#include "../third-party/Catch/single_include/catch.hpp"

TEST_CASE("Test assert", "[tools]")
{
  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(100);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert(23 < 24);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert((14 < 13)?0:1);
  REQUIRE(emp::assert_last_fail == 0);


  // Now here are some that should FAIL
/*  emp_assert(false);
  EMP_TEST_VALUE(emp::assert_last_fail, "1");
  EMP_TEST_VALUE(emp::assert_fail_info.filename, "assert.cc");
  EMP_TEST_VALUE(emp::assert_fail_info.line_num, "31");
  EMP_TEST_VALUE(emp::assert_fail_info.error, "false");
*/
  // if (emp::assert_fail_info.filename != "assert.cc") std::cerr << "Failed case 6!" << std::endl;
  // if (emp::assert_fail_info.line_num != __LINE__ - 3) std::cerr << "Failed case 7!" << std::endl;
  // if (emp::assert_fail_info.error != "false") std::cerr << "Failed case 8!" << std::endl;
}
