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
	
	world1.SetPopStruct_Grow(true);
	world1.InjectAt(6, 3);
	REQUIRE(world1.GetNumOrgs() == 1);
	world1.DoBirth(5, 3);
	REQUIRE(world1.GetNumOrgs() == 1);
	REQUIRE(world1.GetNextOrg(0) == 5);
	world1.Update();
	REQUIRE(world1.GetNumOrgs() == 1);
	REQUIRE(world1.GetOrg(0) == 5);
	
	world1.SetAddInjectFun([](emp::Ptr<int> new_org){ return 6; });
	world1.Inject(9);
	REQUIRE(world1.GetOrg(6) == 9);
	world1.Inject(7);
	REQUIRE(world1.GetOrg(6) == 7);
	
	world1.SetAddBirthFun([](emp::Ptr<int> new_org, emp::WorldPosition parent_pos){ return 4;});
	world1.DoBirth(11, 6);
	REQUIRE(world1.GetOrg(4) == 11);
	
	world1.SetKillOrgFun([&world1](){world1.DoDeath(4); return 4;});
	REQUIRE(world1.GetNumOrgs() == 3);
	world1.DoDeath();
	REQUIRE(world1.GetNumOrgs() == 2);
	
	world1.clear();
	REQUIRE(world1.GetNumOrgs() == 0);
	REQUIRE(world1.size() == 0);
	
	emp::World<double> world2("World 2");
	world2.SetPopStruct_Grid(3, 5, true);
	REQUIRE(world2.GetWidth() == 3);
	REQUIRE(world2.GetHeight() == 5);
	REQUIRE(world2.IsSynchronous());
	world2.MarkSynchronous(false);
	REQUIRE(world2.IsSynchronous() == false);
	world2.MarkSynchronous();
	REQUIRE(world2.HasAttribute("PopStruct"));
	REQUIRE(world2.GetAttribute("PopStruct") == "Grid");
	
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
	
	emp::World<double> world3("World3");
	REQUIRE(world3.GetNumOrgs() == 0);
	emp::Random rnd(1);
	world3.SetRandom(rnd);
	// Figure out how to use InjectRandomOrg
	//world3.InjectRandomOrg(8.2);
	//REQUIRE(world3.GetNumOrgs() == 1);
	
	world3.InjectAt(6.5, 0);
	world3.SetCache();
	REQUIRE(world3.IsCacheOn());
	REQUIRE(world3.CalcFitnessID(0) == 6.5);
	world3.ClearCache();
	
	//world3.CalcFitnessAll();
	std::function<long long unsigned int(double&, emp::Random&)> mutfun = [](double &o, emp::Random& r){ o=o*2; return 1;};
	world3.SetMutFun(mutfun);
	world3.DoMutationsID(0);
	REQUIRE(world3[0] == 13);
	
	world3.InjectAt(3.1, 1);
	world3.InjectAt(8.0, 2);
	world3.DoMutations();
	REQUIRE(world3[0] == 26.0);
	REQUIRE(world3[1] == 6.2);
	REQUIRE(world3[2] == 16.0);
	
	size_t randomID = world3.GetRandomCellID(0, 3);
	bool inRange = ( (randomID < 3) && (randomID >= 0) );
	REQUIRE( inRange );
	
	world3.DoDeath(2);
	world3.DoDeath(1);
	REQUIRE(world3.GetRandomOrg() == 26.0);
	
	emp::vector<size_t> validIDs = world3.GetValidOrgIDs();
	REQUIRE(validIDs.size() == 1);
	REQUIRE(validIDs[0] == 0);
	
	emp::vector<size_t> emptyIDs = world3.GetEmptyPopIDs();
	REQUIRE(emptyIDs.size() == (world3.GetSize() - 1) );
	
	world3.SetAutoMutate();
	world3.SetAddBirthFun([](emp::Ptr<double> new_org, emp::WorldPosition parent_pos){ return parent_pos.GetIndex()+1;});
	world3.DoBirth(1.8, 0);
	// 3.6 because mutate function (defined above) doubles the org
	// SetAutoMutate means the org will mutate before being placed in the world
	REQUIRE(world3[1] == 3.6);
	
	world3.SetAutoMutate(3);
	world3.InjectAt(4.5, 2);
	REQUIRE(world3[2] == 4.5);
	world3.InjectAt(3.3, 3);
	REQUIRE(world3[3] == 6.6);
	
	emp::World<double> world4;
	REQUIRE(world4.size() == 0);
	world4.resize(10);
	REQUIRE(world4.size() == 10);
	REQUIRE( (world4.begin() == world4.end()) );
	
	// discards qualifiers because  world_iterator::bool() is const but calls MakeValid
	// REQUIRE(world4.begin());
	
	world4.InjectAt(2.3, 0);
	REQUIRE( *world4.begin() == 2.3 );
	
	world4.SetGetNeighborFun([](emp::WorldPosition pos){ return emp::WorldPosition(pos.GetIndex()+1); });
	REQUIRE(world4.GetRandomNeighborPos(0).GetIndex() == 1);
	
	REQUIRE(world4.GetPhenotypes().GetSize() == 0);
	std::function<double(double&)> func = [](double& o){ return ((int)o % 2 == 0) ?  o*2.0 : o*0.5; };
	world4.AddPhenotype("trait1", func);
	REQUIRE(world4.GetPhenotypes().GetSize() == 1);
	REQUIRE(world4.GetPhenotypes().Find("trait1") == 0);
	
	// signals?
	// Inject into maybe a full world? try to get else statement coverage
	// Add/Get DataNode
	// Resize #3

}