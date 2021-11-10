/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file assert_warning.cpp
 */

#undef NDEBUG
#define TDEBUG 1

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/assert_warning.hpp"

TEST_CASE("Test assert_warning", "[base]")
{

  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert_warning(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning(100);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning(23 < 24);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning((14 < 13)?0:1);
  REQUIRE(emp::assert_last_fail == 0);

  // Best way to test different behavior of emp_assert_warning based on compiler flags?

  emp_assert_warning(false);
  REQUIRE(emp::assert_last_fail);

  /* not sure why file is assert.h and line_num is 95, is this expected behavior?
  std::string filen = emp::assert_fail_info.filename;
  std::string test_filen = "assert.h";
  REQUIRE(filen.substr(filen.size()-test_filen.size()) == test_filen);
  REQUIRE(emp::assert_fail_info.line_num == 95);
  */

  REQUIRE(emp::assert_fail_info.error == "false");
  emp::assert_clear();
}

TEST_CASE("Another test assert_warning", "[tools]")
{
  // Asserts are tricky to test.  Here are a bunch that should PASS.
  emp_assert_warning(true);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning(100);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning(23 < 24);
  REQUIRE(emp::assert_last_fail == 0);

  emp_assert_warning((14 < 13)?0:1);
  REQUIRE(emp::assert_last_fail == 0);


  // Now here are some that should FAIL
/*  emp_assert_warning(false);
  EMP_TEST_VALUE(emp::assert_last_fail, "1");
  EMP_TEST_VALUE(emp::assert_fail_info.filename, "assert.cc");
  EMP_TEST_VALUE(emp::assert_fail_info.line_num, "31");
  EMP_TEST_VALUE(emp::assert_fail_info.error, "false");
*/
  // if (emp::assert_fail_info.filename != "assert.cc") std::cerr << "Failed case 6!" << std::endl;
  // if (emp::assert_fail_info.line_num != __LINE__ - 3) std::cerr << "Failed case 7!" << std::endl;
  // if (emp::assert_fail_info.error != "false") std::cerr << "Failed case 8!" << std::endl;
}
