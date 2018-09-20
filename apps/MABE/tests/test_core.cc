#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "../core/GenomeWrapper.h"

TEST_CASE("Test GenomeWrapper", "[core]")
{
  int x = 1;
  REQUIRE(x == 1);
}
