//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file is an example for using the re-vamped World template.

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"

int main() {
  emp::World<int> world;
  world.SetPopStruct_Mixed(true);

  // Inject from 100 to 199.
  for (int i = 0; i < 100; i++) { world.Inject(i+200); }

  std::cout << "Start Size = " << world.GetSize() << std::endl << std::endl;
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
            << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
            << std::endl;
  std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
            << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
            << std::endl;

  for (size_t UD = 0; UD < 2; UD++) {

    std::cout << "UD = " << UD << std::endl;

    EliteSelect(world, 50, 4);

    std::cout << "Post EliteSelect(50,4)" << std::endl;
    for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
    std::cout << std::endl;

    std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
              << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
              << std::endl;
    std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
              << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
              << std::endl;

    world.Update();

    std::cout << "Post Update()" << std::endl;
    for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
    std::cout << std::endl;

    std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
              << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
              << std::endl;
    std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
              << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
              << std::endl;
  }

  // world.TournamentSelect(5, 100);
  // world.Update();
  // std::cout << "\nPost-Tourney Size = " << world.GetSize() << std::endl << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl;
  //
  // EliteSelect(world, 10, 10);
  // world.Update();
  // std::cout << "\nPost-Elite Size = " << world.GetSize() << std::endl << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl << std::endl;

}
