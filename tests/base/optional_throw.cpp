/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief TODO.
#include <iostream>
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#define TDEBUG 1
#include "emp/base/optional_throw.hpp"

TEST_CASE("Optional throw" "[asserts]") {

  emp_optional_throw(false);
  REQUIRE(emp::assert_last_fail);

  #define EMP_OPTIONAL_THROW_ON 1
  try {
    emp_optional_throw(false);
  }
  catch (std::runtime_error & error) {
    REQUIRE(std::string(error.what()) == "Internal Error (in always_assert.cpp line 37): false,\nfalse: [0]\n");
  }
}
