//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the pool options for emp::World.h

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  constexpr size_t POP_SIZE = 3600;
  constexpr size_t GENS = 10000;
  const size_t POOL_SIZE = (size_t) std::sqrt(POP_SIZE);
  const size_t NUM_POOLS = (size_t) std::sqrt(POP_SIZE);

  emp::Random random;

  std::function<void(int&,std::ostream &)> print_fun = [](int & val, std::ostream & os) {
    char out_char = '+';
    val %= 63;
    if (val < 10) out_char = (char) ('0' + val);
    else if (val < 36) out_char = (char) ('a' + (val - 10));
    else if (val < 62) out_char = (char) ('A' + (val - 36));
    os << out_char;
  };

  emp::World<int> pool_world(random);
  emp::SetPools(pool_world, NUM_POOLS, POOL_SIZE);
  pool_world.SetPrintFun(print_fun);

  for (size_t i = 0; i < POP_SIZE; i++) pool_world.InjectAt((int)i,i);
  pool_world.PrintGrid();

  for (size_t g = 0; g < GENS; g++) {
    for (size_t i = 0; i < pool_world.GetSize(); ++i) {
      size_t id = random.GetUInt(pool_world.GetSize());
      if (pool_world.IsOccupied(id)) pool_world.DoBirth(pool_world[id], id);
    }
    if (g % 1000 == 0) std::cout << "Generation: " << g << std::endl;
  }

  std::cout << std::endl;
  pool_world.PrintGrid();
  std::cout << "Final Org Counts:\n";
  pool_world.PrintOrgCounts();
  std::cout << std::endl;
}
