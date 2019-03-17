#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "Evolve/World_structure.h"
#include "Evolve/World.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test World structure", "[Evolve]")
{
	emp::World<int> world;
	world.InjectAt(23, 0);
	world.InjectAt(28, 1);
	world.InjectAt(25, 2);
	
	SetPools(world, 3, 1);
	REQUIRE(world.GetSize() == 3);
	REQUIRE(world.GetNumOrgs() == 3);
	REQUIRE(!world.IsSynchronous());
	REQUIRE(world.IsSpaceStructured());
	REQUIRE(world.GetAttribute("PopStruct") == "Pools");
	world.DoBirth(40, 1);
	REQUIRE(world[1] == 40);
	REQUIRE(world.GetNumOrgs() == 3);
	
	SetPools(world, 2, 2, true);
	REQUIRE(world.GetSize() == 4);
	REQUIRE(world.GetNumOrgs() == 3);
	REQUIRE(world.IsSynchronous());
	REQUIRE(world.IsSpaceStructured());
	REQUIRE(world.GetAttribute("PopStruct") == "Pools");
	world.DoBirth(42, 2);
	REQUIRE(world[2] == 25);
	REQUIRE(world.GetNextOrg(2) == 42);
	
	emp::World<int> world1;
	emp::TraitSet<int> ts1;
	std::function<double(int&)> fun = [](int& o){ return o > 50 ? 1.0 : 0.0;};
	ts1.AddTrait(">50", fun);
	std::function<double(int&)> fun2 = [](int& o){ return (o % 2 == 0) ? 0.0 : 1.0;};
	ts1.AddTrait("IsOdd", fun2);
	emp::vector<size_t> ts1_counts{ 1, 2 };
	SetMapElites(world1, ts1, ts1_counts);
	REQUIRE(world1.GetSize() == 2);
	REQUIRE(world1.IsSynchronous() == false);
	REQUIRE(world1.IsSpaceStructured() == false);
	world1.Inject(5);
	REQUIRE(world1[0] == 5);
	REQUIRE(world1.GetNumOrgs() == 1);
}