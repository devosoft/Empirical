/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file World.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <sstream>

#include "emp/base/array.hpp"
#include "emp/bits/BitSet.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/Evolve/Resource.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

constexpr size_t K = 3;
constexpr size_t N = 40;

using BitOrg = emp::BitSet<N>;

TEST_CASE("Test World", "[Evolve]")
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
  world1.RemoveOrgAt(0);
  REQUIRE(world1.GetNumOrgs() == 0);
  world1.InjectAt(5, 0);

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
  world2.SetPopStruct_Grid(1, 2, true);
  REQUIRE(world2.GetWidth() == 1);
  REQUIRE(world2.GetHeight() == 2);
  world2.InjectAt(3.0, 0);
  REQUIRE(world2.GetNumOrgs() == 1);
  world2.DoBirth(2.5, 0);
  REQUIRE(world2[0] == 3.0);
  world2.DoDeath();

  REQUIRE(world2.IsSynchronous());
  world2.MarkSynchronous(false);
  REQUIRE(world2.IsSynchronous() == false);
  world2.MarkSynchronous();
  REQUIRE(world2.HasAttribute("PopStruct"));
  REQUIRE(world2.GetAttribute("PopStruct") == "Grid");
  world2.SetPopStruct_Grid(3, 5, true);

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
  bool inRange = ( (randomID < 3) );
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
  //REQUIRE(world4.GetPhenotypes().Find("trait1") == 0);

  // Inject into maybe a full world? try to get else statement coverage
  // Add/Get DataNode
  // Resize #3
  // GetFile
  // Systematics (broke?)

  // Signals - having issues with clang++ test coverage
  /*emp::World<double> world5;
  world5.Resize(5);
  world5.InjectAt(3.5, 0);
  const emp::SignalControl sglc = world5.GetSignalControl();
  REQUIRE(sglc.GetNumSignals() == 9);
  REQUIRE(sglc.GetNumActions() == 0);

  std::function<void(size_t)> obr = [&world5](size_t w) mutable { world5.InjectAt(5.0, w); };
  world5.OnBeforeRepro(obr);
  world5.SetAddBirthFun([](emp::Ptr<double> o, emp::WorldPosition p){ return p.GetIndex()+1; });
  world5.DoBirth(6.7, 0);
  REQUIRE(world5[0] == 5.0);
  REQUIRE(world5[1] == 6.7);
  REQUIRE(world5.GetNumOrgs() == 2);

  std::function<void(size_t)> ood = [&world5](size_t w) mutable { world5.InjectAt(9.9, (w+1)%world5.size()); };
  world5.OnOrgDeath(ood);
  world5.RemoveOrgAt(1);
  REQUIRE(world5[2] == 9.9);
  REQUIRE(world5.GetNumOrgs() == 2);

  std::function<void(emp::WorldPosition, emp::WorldPosition)> oso = [&world5](emp::WorldPosition o1, emp::WorldPosition o2) mutable { world5.InjectAt(world5[o2.GetIndex()], (o1.GetIndex()+1)%world5.size()); };
  world5.OnSwapOrgs(oso);
  world5.Swap(0, 2);
  REQUIRE(world5[1] == 5.0);
  REQUIRE(world5[2] == 5.0);
  REQUIRE(world5[0] == 9.9);

  std::function<void(double&)> oir = [](double& o) mutable { o = 1.0; };
  world5.OnInjectReady(oir);
  world5.InjectAt(33.0, 3);
  REQUIRE(world5[3] == 1.0);
  */

  emp::World<double> world6;
  world6.resize(5);
  world6.InjectAt(5.0, 0);
  world6.InjectAt(10.0, 1);
  world6.InjectAt(15.0, 2);
  world6.InjectAt(20.0, 3);
  world6.InjectAt(25.0, 4);
  emp::World_iterator<emp::World<double>> w_it = world6.begin();
  emp::World_iterator<emp::World<double>> w_it1 = world6.begin();
  REQUIRE(*w_it == 5.0);
  ++w_it;
  REQUIRE(*w_it == 10.0);
  --w_it;
  REQUIRE(*w_it == 5.0);
  ++w_it;
  ++w_it;
  ++w_it;
  w_it1 = w_it;
  REQUIRE(*w_it1 == 20.0);
  --w_it;
  REQUIRE( (w_it < w_it1) );
  REQUIRE( (w_it <= w_it1) );
  REQUIRE( (w_it1 > w_it) );
  REQUIRE( (w_it1 >= w_it) );
  REQUIRE( (w_it1 != w_it) );
  ++w_it;
  REQUIRE( (w_it == w_it1) );
  REQUIRE( (w_it <= w_it1) );
  REQUIRE( (w_it >= w_it1) );
  REQUIRE( (w_it.begin() == world6.begin()) );
  REQUIRE( (w_it.end() == world6.end()) );
}

TEST_CASE("Test fitness sharing", "[evo]")
{
  size_t POP_SIZE = 100;

  emp::Random random(1);
  emp::World<BitOrg> pop(random);
  pop.SetPopStruct_Mixed(true);
  pop.SetMutFun([](BitOrg & org, emp::Random & r){return 0;});

  // Build a random initial population
  for (size_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < N; j++) next_org[j] = 0;
    pop.Inject(next_org);
    REQUIRE( pop.GetDominantOrg() == next_org );
  }
  REQUIRE( pop.GetDominantInfo().second == POP_SIZE );



  // Setup the (shared) fitness function.
  pop.SetSharedFitFun( [](BitOrg &org){ return 10 + N - org.CountOnes(); },
                       [](BitOrg& org1, BitOrg& org2){ return (double)(org1.XOR(org2)).CountOnes();},
                       10, 1 );


  REQUIRE(pop.CalcFitnessID(0) == 0.50);


  BitOrg next_org;
  for (size_t j = 0; j < N; j++) next_org[j] = 1;
  pop.InjectAt(next_org, POP_SIZE-1);
  pop.InjectAt(next_org, POP_SIZE-2);
  pop.InjectAt(next_org, POP_SIZE-3);
  pop.InjectAt(next_org, POP_SIZE-4);
  pop.InjectAt(next_org, POP_SIZE-5);
  REQUIRE( pop.GetDominantInfo().second == POP_SIZE - 5 );

  REQUIRE(pop.CalcFitnessID(0) == Approx(0.526316));
  REQUIRE(pop.CalcFitnessID(POP_SIZE-1) == 2);

  // Check neighbor function works for mixed
  REQUIRE(pop.GetValidNeighborOrgIDs(21).size() == POP_SIZE);
  REQUIRE(pop.IsNeighbor(21, 20));

  // Run a tournament...
  emp::TournamentSelect(pop, 5, POP_SIZE);
  pop.Update();

  REQUIRE( pop.GetDominantInfo().second < POP_SIZE - 5 );
  // REQUIRE(pop.CalcFitnessID(0) == Approx(0.322581));

  pop.SetFitFun([](BitOrg &org){ return N - org.CountOnes(); });

  emp::vector<std::function<double(BitOrg&)> > fit_funs;

  fit_funs.push_back([](BitOrg &org){ return org.CountOnes(); });
  fit_funs.push_back([](BitOrg &org){ return org[0]; });
  fit_funs.push_back([](BitOrg &org){ return 1 - org[0]; });

  // pop.SetCache(true);

  emp::EcoSelect(pop, fit_funs, 1000, 5, POP_SIZE);

  // TODO: Come up with better tests for EcoSelect

  std::cout << "--- Grid example ---\n";

  POP_SIZE = 400;

  std::function<void(int &, std::ostream &)> print_fun = [](int & val, std::ostream & os) {
    val %= 63;
    if (val < 10) os << (char) ('0' + val);
    else if (val < 36) os << (char) ('a' + (val - 10));
    else if (val < 62) os << (char) ('A' + (val - 36));
    else os << '+';
  };

  emp::World<int> grid_world(random);
  grid_world.SetMutFun([](int & org, emp::Random & r){return 0;});
  const size_t side = (size_t) std::sqrt(POP_SIZE);
  grid_world.SetPopStruct_Grid(side, side);
  grid_world.SetPrintFun(print_fun);

  emp_assert(grid_world.GetSize() == POP_SIZE); // POP_SIZE needs to be a perfect square.

  grid_world.InjectAt(30, side+1);
  grid_world.InjectAt(4, side*(side+1)/2);
  grid_world.PrintGrid();

  auto fit_fun = [](int & org){ return (double) org; };
  grid_world.SetSharedFitFun(fit_fun, [](int & a, int & b){ return (double) (a>b)?(a-b):(b-a); }, 3, 1);
  RouletteSelect(grid_world, 500);

  std::cout << std::endl;
  grid_world.PrintGrid();
  //std::cout << "Final Org Counts:\n";
  //   grid_world.PrintOrgCounts(print_fun);
  //   std::cout << std::endl;

}

TEST_CASE("Test 3D population structure", "[Evolve]")
{
  emp::World<int> world;
  world.SetPopStruct_3DGrid(5,4,3);
  CHECK(world.GetWidth() == 5);
  CHECK(world.GetHeight() == 4);
  CHECK(world.GetDepth() == 3);
  CHECK(world.GetSize() == 5*4*3);

  int org1 = 5;

  world.InjectAt(org1, 0);

  // Test lower bounds
  emp::WorldPosition neigh = world.GetRandomNeighborPos(0);
  emp::vector<size_t> legal_neighbors = {1,5,6,20,21,25,26};
  // std::cout << neigh.GetIndex() << " " << emp::to_string(legal_neighbors) << std::endl;
  CHECK(emp::Has<size_t>(legal_neighbors, neigh.GetIndex()));

  // Test middle of grid
  neigh = world.GetRandomNeighborPos(26);
  legal_neighbors = {0,1,2, 5,6,7,10,11,12,20, 21, 22, 25, 27, 30, 31, 32, 40, 41, 42, 45, 46, 47, 50, 51, 52};
  // std::cout << neigh.GetIndex() << " " << emp::to_string(legal_neighbors) << std::endl;
  CHECK(emp::Has<size_t>(legal_neighbors, neigh.GetIndex()));

  // Test upper bounds
  neigh = world.GetRandomNeighborPos(59);
  legal_neighbors = {58, 54, 53, 39, 38, 34, 33};
  // std::cout << neigh.GetIndex() << " " << emp::to_string(legal_neighbors) << std::endl;
  CHECK(emp::Has<size_t>(legal_neighbors, neigh.GetIndex()));

  // Test is_neighbor
  CHECK(world.IsNeighbor(5, 6));
  CHECK(world.IsNeighbor(0, 20));
  CHECK(world.IsNeighbor(0, 21));
  CHECK(world.IsNeighbor(0, 5));
  CHECK(!world.IsNeighbor(0, 22));

  world.Inject(org1);
  world.DoBirth(org1, 0);
  world.DoDeath();

  // test synchronous
  world.Clear();
  world.SetPopStruct_3DGrid(5,4,3, true);
  world.InjectAt(org1, 0);
  emp::WorldPosition pos = world.DoBirth(org1, 0);
  legal_neighbors = {1,5,6,20,21,25,26};
  CHECK(emp::Has<size_t>(legal_neighbors, pos.GetIndex() ));
  CHECK(pos.GetPopID() == 1);
  CHECK(world.GetNumOrgs() == 1);
  world.Update();
  CHECK(world.GetNumOrgs() == 1);
  world.Update();
  CHECK(world.GetNumOrgs() == 0);

}

TEST_CASE("Test GetValidNeighborOrgIDs on Grid", "[Evolve]")
{
  emp::Random random(1);
  std::function<void(int &, std::ostream &)> print_fun = [](int & val, std::ostream & os) {
    val %= 63;
    if (val < 10) os << (char) ('0' + val);
    else if (val < 36) os << (char) ('a' + (val - 10));
    else if (val < 62) os << (char) ('A' + (val - 36));
    else os << '+';
  };

  emp::World<int> grid_world(random);
  grid_world.SetPopStruct_Grid(5, 10);
  grid_world.Resize(5, 10);
  grid_world.SetPrintFun(print_fun);

  grid_world.InjectAt(30, 12);
  grid_world.InjectAt(31, 13);
  grid_world.InjectAt(34, 7);
  grid_world.InjectAt(30, 0);
  grid_world.InjectAt(32, 49);

  // Check neighbor function works for grid
  std::cout << "Testing Neighbor functions in Grid\n";
  grid_world.PrintGrid();
  emp::vector<size_t> valid_neighbors = {7, 13};
  REQUIRE(grid_world.IsNeighbor(12, 13));
  REQUIRE(grid_world.IsNeighbor(13, 12));
  REQUIRE(grid_world.GetValidNeighborOrgIDs(12) == valid_neighbors);
  REQUIRE(grid_world.IsNeighbor(0, 49));

  std::cout << std::endl;
}
