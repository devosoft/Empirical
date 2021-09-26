/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file ShrinkPop.cpp
 *  @brief This file explores the grid options for emp::World.hpp
 */

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  constexpr size_t POP_SIZE = 3600;
  constexpr size_t GENS = 10000;
  const size_t POP_SIDE = (size_t) std::sqrt(POP_SIZE);

  emp::Random random;

  std::function<void(int&,std::ostream &)> print_fun = [](int & val, std::ostream & os) {
    char out_char = '+';
    val %= 63;
    if (val < 10) out_char = (char) ('0' + val);
    else if (val < 36) out_char = (char) ('a' + (val - 10));
    else if (val < 62) out_char = (char) ('A' + (val - 36));
    os << out_char;
  };

  emp::World<int> grid_world(random);
  grid_world.SetPopStruct_Grid(POP_SIDE, POP_SIDE);
  grid_world.SetPrintFun(print_fun);

  for (size_t i = 0; i < POP_SIZE; i++) grid_world.InjectAt((int)i,i);

  // What does the grid look like after inject?
  std::cout << "BEFORE SerialTransfer(0.01):" << std::endl;
  grid_world.PrintGrid();


  grid_world.SerialTransfer(0.01);
//   for (size_t g = 0; g < GENS; g++) {
//     for (size_t i = 0; i < grid_world.GetSize(); ++i) {
//       size_t id = random.GetUInt(grid_world.GetSize());
//       if (grid_world.IsOccupied(id)) grid_world.DoBirth(grid_world[id], id);
//     }
//     if (g % 1000 == 0) std::cout << "Generation: " << g << std::endl;
//   }

  std::cout << std::endl;
  std::cout << "AFTER SerialTransfer(0.01):" << std::endl;
  grid_world.PrintGrid();
  std::cout << "Final Grid Org Counts:\n";
  grid_world.PrintOrgCounts();
  std::cout << std::endl;





  emp::World<int> mass_world(random);
  grid_world.SetPopStruct_Mixed();
  grid_world.SetPrintFun(print_fun);

  for (size_t i = 0; i < POP_SIZE; i++) grid_world.InjectAt((int)i%10,i);

  // What does the grid look like after inject?
  std::cout << "Mass action, BEFORE Bottlneck(20):" << std::endl;
  grid_world.PrintOrgCounts();

  grid_world.DoBottleneck(20);

  std::cout << std::endl;
  std::cout << "Mass action, AFTER Bottleneck" << std::endl;
  grid_world.PrintOrgCounts();
  std::cout << std::endl;
}
