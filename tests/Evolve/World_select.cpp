/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file World_select.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/Evolve/World.hpp"
#include "emp/Evolve/World_select.hpp"

TEST_CASE("Test World select", "[Evolve]")
{
  emp::World<int> world;
  world.InjectAt(0, 0);
  world.InjectAt(2, 1);
  world.InjectAt(4, 2);
  world.InjectAt(1, 3);
  REQUIRE(world.GetNumOrgs() == 4);
  REQUIRE(world.size() == 4);
  std::function<double(int&)> isodd = [](int& o){ return (o % 2 == 0) ? 0.0 : 1.0;};
  world.AddPhenotype("IsOdd", isodd);
  world.SetPopStruct_Grow(true);
  emp::RandomSelect(world);
  world.Update();
  REQUIRE(world.GetNumOrgs() == 1);
  world.Inject(3);
  world.Inject(7);
  world.Inject(9);
  world.Inject(5);

  std::function<double(const int&)> isoddconst = [](const int& o){ return (o % 2 == 0) ? 0.0 : 1.0;};
  std::function<double(const int&)> remainthree = [](const int& o){ return o % 3; };
  emp::vector<std::function<double(const int&)>> listOfFunctions;
  listOfFunctions.push_back(isoddconst);
  listOfFunctions.push_back(remainthree);
  emp::LexicaseSelect(world, listOfFunctions);
  REQUIRE(world.GetNumOrgs() == 5);
  world.Update();
  REQUIRE(world.GetNumOrgs() == 1);
  REQUIRE(world[0] == 5);
  world.Inject(3);
  world.Inject(7);
  world.Inject(9);

  emp::OptimizedLexicaseSelect(world, listOfFunctions);
  REQUIRE(world.GetNumOrgs() == 4);
  world.Update();
  REQUIRE(world.GetNumOrgs() == 1);
}
