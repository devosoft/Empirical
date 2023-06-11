/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file Fitness_Share_NK.cpp
 *  @brief This file uses the Fitness Sharing functionality defined in evo::World.hpp
 */

#include <iostream>

#include "emp/bits/BitSet.hpp"
#include "emp/Evolve/NK-const.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"


constexpr size_t K = 3;
constexpr size_t N = 50;

using BitOrg = emp::BitSet<N>;

int main()
{
  size_t POP_SIZE = 100;
  size_t UD_COUNT = 1000;

  emp::Random random;
  emp::evo::NKLandscapeConst<N,K> landscape(random);
  emp::World<BitOrg> pop(random);
  pop.SetPopStruct_Mixed(true);
  pop.SetCache();

  // Build a random initial population
  for (size_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (size_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Inject(next_org);
  }

  // Setup the (shared) fitness function.
  pop.SetSharedFitFun( [&landscape](BitOrg &org){ return landscape.GetFitness(org); },
                       [](BitOrg& org1, BitOrg& org2){ return (double)(org1.XOR(org2)).CountOnes();},
                       10, 1 );

  pop.SetMutFun( [](BitOrg & org, emp::Random & random){
    size_t count = 0;
    if (random.P(0.5)) { org[random.GetUInt(N)].Toggle(); count++; }
    if (random.P(0.5)) { org[random.GetUInt(N)].Toggle(); count++; }
    if (random.P(0.5)) { org[random.GetUInt(N)].Toggle(); count++; }
    return count;
  } );

  // Loop through updates
  for (size_t ud = 0; ud < UD_COUNT; ud++) {
    // Run a tournament...
    emp::TournamentSelect(pop, 5, POP_SIZE-1);
    pop.Update();
    pop.DoMutations();
  }


  std::cout << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;



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
