#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/NullStream.h"

#include <sstream>
#include <iostream>



TEST_CASE("Test NullStream", "[tools]")
{
  emp::NullStream ns;
  ns << "abcdefg";
  ns << std::endl;
  ns << 123;
  ns << 123.456;
  ns.flush();
}
