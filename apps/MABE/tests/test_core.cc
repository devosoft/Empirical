#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "../core/GenomeWrapper.h"
#include <utility>

// A test genome with no special functions.
struct TestGenomeA { std::string foo = "This is my member var."; };

// A test genome that uses all special functions.
struct TestGenomeB {
  std::string name = "My Name";

  std::string GetClassName() const { return "TestGenomeB"; }
};

TEST_CASE("Test GenomeWrapper", "[core]")
{
  mabe::GenomeWrapper<TestGenomeA> genA;
  mabe::GenomeWrapper<TestGenomeB> genB;

  REQUIRE(genA.GetClassName() == "NoName");
  REQUIRE(genB.GetClassName() == "TestGenomeB");
}
