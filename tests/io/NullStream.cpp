/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file NullStream.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/NullStream.hpp"

TEST_CASE("Test NullStream", "[io]")
{
  emp::NullStream ns;
  ns << "abcdefg";
  ns << std::endl;
  ns << 123;
  ns << 123.456;
  ns.flush();
}

TEST_CASE("Test nout", "[io]")
{
  emp::nout << "abcdefg";
  emp::nout << std::endl;
  emp::nout << 123;
  emp::nout << 123.456;
  emp::nout.flush();
}
