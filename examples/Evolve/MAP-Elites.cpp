//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the MAP-Elites selection scheme.
//
//  In this example, we will be evolving 4-digit integers.
//  The two traits measured are nunmber of bits and value mod 31.

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  constexpr size_t GENS = 1000;

  // Organisms are unsigned ints.
  using org_t = uint64_t;
  constexpr org_t MIN_ORG = 0;
  constexpr org_t MAX_ORG = 8192;
  constexpr org_t MAX_ORG_DIFF = MAX_ORG - MIN_ORG;

std::cout << "START!" << std::endl;

  emp::Random random(1);
  emp::World<org_t> map_world(random);

std::cout << "World build." << std::endl;

  // Fitness = value; trait 1 = num bits; trait 2 = value mod 31
  std::function<double(org_t &)> fit_fun =    [](org_t & val){ return (double) val; };
  std::function<double(org_t &)> trait1_fun = [](org_t & val){ return (double) emp::count_bits(val); };
  std::function<double(org_t &)> trait2_fun = [](org_t & val){ return (double) (val % 31); };

  map_world.SetFitFun(fit_fun);
  map_world.AddPhenotype("Num Bits", trait1_fun, 0, 14);
  map_world.AddPhenotype("Mod 31", trait2_fun, 0, 31);

std::cout << "Phenotype functions in place." << std::endl;

  emp::SetMapElites(map_world, {14,31});

std::cout << "Setup MAP-Elites" << std::endl;

  // Setup the print function to output the appropriate number of characters.
  std::function<void(org_t&,std::ostream &)> print_fun = [](org_t & val, std::ostream & os) {
    std::string out_str = emp::to_string(val);
    while (out_str.size() < 4) out_str = emp::to_string('.', out_str);
    os << out_str;
  };
  map_world.SetPrintFun(print_fun);


std::cout << "Setup print functions." << std::endl;

  // Start off world with random organism.
  map_world.Inject(random.GetUInt64(MAX_ORG_DIFF/4));
  map_world.PrintGrid(std::cout, "----");

  for (size_t g = 0; g <= GENS; g++) {
    for (size_t i = 0; i < map_world.GetSize(); ++i) {
      size_t id = random.GetUInt(map_world.GetSize());
      if (map_world.IsOccupied(id)) {
        org_t offspring = map_world[id] + random.GetUInt64(200) - 100;
        if (offspring > MAX_ORG) continue; // Invalid mutation!  No birth.
        map_world.DoBirth(offspring , id);
      }
    }
    if (g % 50 == 0) {
      std::cout << "UD: " << g << std::endl;
      map_world.PrintGrid(std::cout, "----");
    }
  }


  // std::cout << "Final Org Counts:\n";
  // map_world.PrintOrgCounts();
  std::cout << std::endl;
}
