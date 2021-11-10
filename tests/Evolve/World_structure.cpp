/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file World_structure.cpp
 */

#define EMP_TDEBUG

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/Evolve/World.hpp"
#include "emp/Evolve/World_structure.hpp"

TEST_CASE("Test World structure", "[Evolve]")
{
  // SetPools (set pools on a world that is full?)
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
  world.InjectAt(43, 0);
  REQUIRE(world[0] == 43);
  REQUIRE(world.GetNumOrgs() == 3);
  world.DoDeath();
  REQUIRE(world.GetNumOrgs() == 2);
  world.Inject(48);
  REQUIRE(world.GetNumOrgs() == 3);

  SetPools(world, 2, 2, true);
  REQUIRE(world.GetSize() == 4);
  REQUIRE(world.GetNumOrgs() == 3);
  REQUIRE(world.IsSynchronous());
  REQUIRE(world.IsSpaceStructured());
  REQUIRE(world.GetAttribute("PopStruct") == "Pools");
  world.DoBirth(42, 2);
  REQUIRE(world[2] != 42);
  REQUIRE(world.GetNextOrg(2) == 42);

  // Set Elites
  emp::World<int> world1;
  emp::TraitSet<int> ts1;
  std::function<double(int&)> fun = [](int& o){ return o > 50 ? 1.0 : 0.0;};
  ts1.AddTrait(">50", fun);
  std::function<double(int&)> fun2 = [](int& o){ return (o % 2 == 0) ? 0.0 : 1.0;};
  ts1.AddTrait("IsOdd", fun2);
  emp::vector<size_t> ts1_counts{ 1, 1 };
  SetMapElites(world1, ts1, ts1_counts);
  REQUIRE(world1.GetAttribute("PopStruct") == "MapElites");
  REQUIRE(world1.GetSize() == 1);
  REQUIRE(world1.IsSynchronous() == false);
  REQUIRE(world1.IsSpaceStructured() == false);
  world1.Inject(5);
  REQUIRE(world1[0] == 5);
  REQUIRE(world1.GetNumOrgs() == 1);
  world1.DoBirth(51,0);
  REQUIRE(world1[0] == 51);
  world1.DoBirth(7,0);
  REQUIRE(world1[0] == 51);
  world1.DoDeath();
  REQUIRE(world1.GetNumOrgs() == 0);

  #ifdef EMP_TDEBUG
  REQUIRE(world1.GetRandomNeighborPos(0).GetIndex() == 0);
  #endif

  emp::World<int> world2;
  world2.Resize(10);
  REQUIRE(world2.GetSize() == 10);
  SetMapElites(world2, ts1);
  REQUIRE(world2.GetAttribute("PopStruct") == "MapElites");
  REQUIRE(world2.GetSize() == 9);

  emp::World<int> world2_1;
  world2_1.Resize(5);
  world2_1.AddPhenotype(">50", fun);
  emp::vector<size_t> trait_counts;
  trait_counts.push_back(world2_1.GetSize());
  SetMapElites(world2_1, trait_counts);
  REQUIRE(world2_1.size() == 5);

  emp::World<int> world3;
  world3.Resize(10);
  REQUIRE(world3.GetSize() == 10);
  emp::TraitSet<int> ts2;
  ts2.AddTrait("IsOdd", fun2);
  SetMapElites(world3, ts2);
  REQUIRE(world3.GetAttribute("PopStruct") == "MapElites");
  REQUIRE(world3.GetSize() == 10);

  emp::World<int> world4;
  world4.Resize(5);
  world4.AddPhenotype(">50", fun);
  SetMapElites(world4);
  REQUIRE(world4.GetAttribute("PopStruct") == "MapElites");
  REQUIRE(world4.GetSize() == 5);

  emp::World<int> world5;
  world5.Resize(2);
  world5.InjectAt(11, 0);
  world5.AddPhenotype("IsOdd", fun2);
  SetDiverseElites(world5, 2);
  REQUIRE(world5.GetAttribute("PopStruct") == "DiverseElites");
  REQUIRE(world5.GetSize() == 2);
  REQUIRE(world5.IsSynchronous() == false);
  REQUIRE(world5.IsSpaceStructured() == false);

  #ifdef EMP_TDEBUG
  REQUIRE(world5.GetRandomNeighborPos(0).GetIndex() == 0);
  #endif

  world5.InjectAt(33, 1);
  REQUIRE(world5[1] == 33);
  REQUIRE(world5.GetNumOrgs() == 2);
  world5.DoDeath();
  REQUIRE(world5.GetNumOrgs() == 1);
  REQUIRE(world5[0] == 33);
  world5.DoBirth(22, 0);
  REQUIRE(world5.GetNumOrgs() == 2);
  REQUIRE(world5[1] == 22);

  // World_MinDistInfo
  emp::World<int> world6;
  world6.InjectAt(4, 0);
  world6.InjectAt(7, 1);
  world6.InjectAt(9, 2);
  REQUIRE(world6.GetSize() == 3);
  emp::World_MinDistInfo<int> w6_distInfo(world6, ts2);
  REQUIRE(w6_distInfo.CalcDist(0, 1) == 1.0);
  REQUIRE(w6_distInfo.CalcDist(1, 2) == 0.0); // both odd
  REQUIRE(w6_distInfo.is_setup == false);
  w6_distInfo.Setup();
  REQUIRE(w6_distInfo.is_setup);
  REQUIRE(w6_distInfo.distance.size() == world6.GetSize());
  REQUIRE(w6_distInfo.CalcBin(0) == 2);
  REQUIRE(w6_distInfo.bin_ids[2].size() == 3);
  REQUIRE(w6_distInfo.bin_ids[0].size() == 0);
  REQUIRE(w6_distInfo.bin_ids[1].size() == 0);
  REQUIRE(w6_distInfo.nearest_id[0] == 1);
  REQUIRE(w6_distInfo.distance[0] == 1.0);

  REQUIRE(w6_distInfo.distance.size() == 3);
  REQUIRE(w6_distInfo.OK());
  world6.InjectAt(11, 0);
  REQUIRE(w6_distInfo.distance[0] == 1.0);
  w6_distInfo.Update(0);
  REQUIRE(w6_distInfo.distance[0] == 0.0);
  REQUIRE(w6_distInfo.distance.size() == 3);

  w6_distInfo.Clear();
  REQUIRE(w6_distInfo.distance.size() == 0);

  // WorldPosition
  emp::WorldPosition worldPos(1, 0);
  REQUIRE(worldPos.GetIndex() == 1);
  REQUIRE(worldPos.GetPopID() == 0);
  worldPos.SetActive();
  REQUIRE(worldPos.IsActive());
  worldPos.SetPopID(1);
  REQUIRE(!worldPos.IsActive());
  REQUIRE(worldPos.GetPopID() == 1);
  worldPos.MarkInvalid();
  REQUIRE(!worldPos.IsValid());
}
