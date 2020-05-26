#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <iostream>

#include "base/array.h"
#include "Evolve/NK-const.h"
#include "Evolve/World.h"
#include "Evolve/Resource.h"
#include "Evolve/NK.h"

#include "tools/BitSet.h"
#include "tools/Random.h"
#include "tools/string_utils.h"

constexpr size_t K = 3;
constexpr size_t N = 40;

using BitOrg = emp::BitSet<N>;

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

  // Run a tournament...
  emp::TournamentSelect(pop, 5, POP_SIZE);
  pop.Update();

  REQUIRE( pop.GetDominantInfo().second < POP_SIZE - 5 );
  REQUIRE(pop.CalcFitnessID(0) == Approx(0.322581));

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
  std::cout << "Final Org Counts:\n";
  //   grid_world.PrintOrgCounts(print_fun);
  //   std::cout << std::endl;

}


TEST_CASE("Test resources", "[evo]")
{
  size_t POP_SIZE = 100;

  emp::Random random(1);
  emp::World<BitOrg> pop(random);
  pop.SetPopStruct_Mixed(true);
  pop.SetMutFun([](BitOrg & org, emp::Random & r){return 0;});

  emp::vector<emp::Resource> resources;
  resources.push_back(emp::Resource(100, 100, .01));
  resources.push_back(emp::Resource(100, 100, .01));
  resources.push_back(emp::Resource(100, 100, .01));

  pop.OnUpdate([&resources](int ud){
      for (emp::Resource& res : resources) {
          res.Update();
      }
  });

  REQUIRE(resources[0].GetAmount() == 100);

  pop.Update();

  REQUIRE(resources[0].GetAmount() == Approx(199.0));

  pop.Update();
  pop.Update();
  pop.Update();
  pop.Update();
  pop.Update();

  // Build a random initial population
  for (size_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < N; j++) next_org[j] = 0;
    pop.Inject(next_org);
  }

  pop.SetFitFun([](BitOrg &org){ return 10; });


  emp::vector<std::function<double(BitOrg&)> > fit_funs;

  fit_funs.push_back([](BitOrg &org){ return org.CountOnes()/N; });
  fit_funs.push_back([](BitOrg &org){ return org[0]; });
  fit_funs.push_back([](BitOrg &org){ return 1 - org[0]; });

  emp::ResourceSelect(pop, fit_funs, resources, 5, POP_SIZE);

  REQUIRE(resources[0].GetAmount() == Approx(779.346));
  REQUIRE(resources[1].GetAmount() == Approx(779.346));
  REQUIRE(resources[2].GetAmount() == Approx(617.265));

}
