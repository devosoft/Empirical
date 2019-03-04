#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "Evolve/World_structure.h"
#include "Evolve/World.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test World structure", "[Evolve]")
{
	emp::World<int> world1;
	REQUIRE(world1.GetSize() == 0);
	REQUIRE(world1.GetNumOrgs() == 0);
	REQUIRE(world1.GetFullPop().size() == 0);
	
	world1.InjectAt(5, 0);
	REQUIRE(world1.GetOrg(0) == 5);
	REQUIRE(world1.GetNumOrgs() == 1);
	
	world1.InjectAt(3, 1);
	REQUIRE(world1.GetOrg(1) == 3);
	REQUIRE(world1.GetNumOrgs() == 2);
	
	world1.Swap(0, 1);
	REQUIRE(world1.GetOrg(0) == 3);
	REQUIRE(world1.GetOrg(1) == 5);
	
	world1.DoDeath(1);
	REQUIRE(world1.GetNumOrgs() == 1);
	
	world1.Reset();
	REQUIRE(world1.GetNumOrgs() == 0);
	
	emp::World<double> world2;
	world2.SetPopStruct_Grid(3, 5, true);
	REQUIRE(world2.GetWidth() == 3);
	REQUIRE(world2.GetHeight() == 5);
	REQUIRE(world2.IsSynchronous());
	world2.MarkSynchronous(false);
	REQUIRE(world2.IsSynchronous() == false);
	// Get Attribute throws assert error "Has(attributes, name)"
	//REQUIRE(world2.GetAttribute("PopStruct") == "Grid");
	
	// AHhhhhhhhhh
	world2.InjectAt(6.1, 0);
	world2.InjectAt(3.5, 3);
	world2.InjectAt(0.9, 6);
	
	REQUIRE(world2[0] == 6.1);
	REQUIRE(world2[3] == 3.5);
	REQUIRE(world2[6] == 0.9);
	REQUIRE(world2.GetOrg(0,0) == 6.1);
	REQUIRE(world2.GetOrg(0,1) == 3.5);
	REQUIRE(world2.GetOrg(0,2) == 0.9);
	REQUIRE(world2.GetGenome(world2.GetOrg(3)) == 3.5);
	
	REQUIRE(world2.IsSpaceStructured());
	world2.MarkSpaceStructured(false);
	REQUIRE(world2.IsSpaceStructured() == false);
	world2.MarkSpaceStructured();
	
	REQUIRE(world2.IsPhenoStructured() == false);
	world2.MarkPhenoStructured();
	REQUIRE(world2.IsPhenoStructured());

	std::function<double(double&)> calc_info = [](double o){ return o; };
	emp::Systematics<double,double> sys1(calc_info);
	emp::Ptr<emp::Systematics<double,double>> w2_sys1(&sys1);
	// When trying to add systematic to world program gets a seg fault
	// think it may be caused because world is trying to delete the ptr to
	// the systematic but the ptr has already been deleted by the Ptr destructor?
	//world2.AddSystematics(w2_sys1, "sys1");
	//std::cout << world2.GetSystematics() << std::endl;
	//REQUIRE(world2.GetSystematics("sys1") == w2_sys1);
	
	// Inject into maybe a full world? try to get else statement coverage
	// InjectRandomOrg
}