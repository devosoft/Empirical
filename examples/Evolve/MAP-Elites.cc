//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the MAP-Elites selection scheme.
//
//  In this example, we will be evolving 

#include <iostream>

#include "Evolve/World.h"
#include "tools/Random.h"
#include "tools/string_utils.h"

int main()
{
  constexpr size_t GENS = 10000;

  // Organisms are unsigned ints.
  using org_t = uint64_t;
  org_t MIN_ORG = 0;
  org_t MAX_ORG = 8192;

  emp::Random random;
  emp::World<org_t> map_world(random);

  // Fitness = value; trait 1 = num bits; trait 2 = value mod 31
  std::function<double(org_t &)> fit_fun =    [](org_t & val){ return (double) val; };
  std::function<double(org_t &)> trait1_fun = [](org_t & val){ return (double) emp::count_bits(val); };
  std::function<double(org_t &)> trait2_fun = [](org_t & val){ return (double) (val % 31); };

  map_world.SetFitFun(fit_fun);
  map_world.AddPhenotype("Num Bits", trait1_fun, 0, 13);
  map_world.AddPhenotype("Mod 31", trait2_fun, 0, 31);

  emp::SetMapElites(map_world, {13,31});

  // Setup the print function to output the appropriate number of characters.
  std::function<void(org_t&,std::ostream &)> print_fun = [](org_t & val, std::ostream & os) {
    std::string out_str = emp::to_string(val);
    while (out_str.size() < 4) out_str = emp::to_string('.', out_str);
    os << out_str;
  };
  map_world.SetPrintFun(print_fun);


  // Start off world with random organism.
  map_world.Inject(random.GetUInt64(MIN_ORG, MAX_ORG))
  map_world.PrintGrid();

  for (size_t g = 0; g < GENS; g++) {
    for (size_t i = 0; i < map_world.GetSize(); ++i) {
      size_t id = random.GetUInt(map_world.GetSize());
      if (map_world.IsOccupied(id)) map_world.DoBirth(map_world[id], id);
    }
  }

  std::cout << std::endl;
  map_world.PrintGrid();
  std::cout << "Final Org Counts:\n";
  map_world.PrintOrgCounts();
  std::cout << std::endl;
}
