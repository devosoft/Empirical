//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the RouletteSelect() function

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  constexpr size_t POP_SIZE = 400;
  // constexpr size_t GENS = 10000;

  emp::Random random;

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
  grid_world.SetFitFun(fit_fun);
  RouletteSelect(grid_world, 500);

  std::cout << std::endl;
  grid_world.PrintGrid();
  std::cout << "Final Org Counts:\n";
//   grid_world.PrintOrgCounts(print_fun);
//   std::cout << std::endl;
}
