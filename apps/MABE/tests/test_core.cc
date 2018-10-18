#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <utility>

#include "tools/Random.h"

#include "../core/GenomeWrapper.h"
#include "../core/BrainWrapper.h"

// A test genome with no special functions.
struct TestGenomeA { std::string foo = "This is my member var."; };

// A test genome that uses all special functions.
struct TestGenomeB {
  std::string name = "Start Name";

  std::string GetClassName() const { return "TestGenomeB"; }
  bool Randomize(emp::Random &) { name = "Randomized!"; return true; }
  void OnBeforeRepro() { name = "BeforeRepro!"; }
};


TEST_CASE("Test GenomeWrapper", "[core]")
{
  emp::Random random(1);

  mabe::GenomeWrapper<TestGenomeA> genA;
  mabe::GenomeWrapper<TestGenomeB> genB;

  REQUIRE(genA.GetClassName() == "Unnamed Genome");
  REQUIRE(genB.GetClassName() == "TestGenomeB");

  REQUIRE(genB.name == "Start Name");
  REQUIRE(genA.Randomize(random) == false );
  REQUIRE(genB.Randomize(random) == true );
  REQUIRE(genB.name == "Randomized!");

  genA.OnBeforeRepro();
  genB.OnBeforeRepro();
  REQUIRE(genB.name == "BeforeRepro!");
}


// A test genome with no special functions.
struct TestBrainA { std::string foo = "This is my member var."; };

// A test genome that uses all special functions.
struct TestBrainB {
  std::string name = "Start Name";

  std::string GetClassName() const { return "TestBrainB"; }
  bool Randomize(emp::Random &) { name = "Randomized!"; return true; }
  void OnBeforeRepro() { name = "BeforeRepro!"; }
};


TEST_CASE("Test BrainWrapper", "[core]")
{
  emp::Random random(1);

  mabe::BrainWrapper<TestBrainA> genA;
  mabe::BrainWrapper<TestBrainB> genB;

  REQUIRE(genA.GetClassName() == "Unnamed Brain");
  REQUIRE(genB.GetClassName() == "TestBrainB");

  REQUIRE(genB.name == "Start Name");
  REQUIRE(genA.Randomize(random) == false );
  REQUIRE(genB.Randomize(random) == true );
  REQUIRE(genB.name == "Randomized!");

  genA.OnBeforeRepro();
  genB.OnBeforeRepro();
  REQUIRE(genB.name == "BeforeRepro!");
}
