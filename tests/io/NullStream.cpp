#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/NullStream.hpp"

#include <sstream>
#include <iostream>



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
