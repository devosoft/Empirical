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

  std::cout << "Start Size = " << world.GetSize() << '\n' << '\n';
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << '\n';

  std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
            << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
            << '\n';
  std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
            << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
            << '\n';

  for (size_t UD = 0; UD < 2; UD++) {

    std::cout << "UD = " << UD << '\n';

    EliteSelect(world, 50, 4);

    std::cout << "Post EliteSelect(50,4)" << '\n';
    for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
    std::cout << '\n';

    std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
              << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
              << '\n';
    std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
              << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
              << '\n';

    world.Update();

    std::cout << "Post Update()" << '\n';
    for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
    std::cout << '\n';

    std::cout << "Num Ptr<int> = " << emp::Ptr<int>::DebugInfo().current
              << "  (total = " << emp::Ptr<int>::DebugInfo().total << ")"
              << '\n';
    std::cout << "Num Ptr<emp::Taxon<int>> = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().current
              << "  (total = " << emp::Ptr<emp::Taxon<int>>::DebugInfo().total << ")"
              << '\n';
  }

  // world.TournamentSelect(5, 100);
  // world.Update();
  // std::cout << "\nPost-Tourney Size = " << world.GetSize() << '\n' << '\n';
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << '\n';
  //
  // EliteSelect(world, 10, 10);
  // world.Update();
  // std::cout << "\nPost-Elite Size = " << world.GetSize() << '\n' << '\n';
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << '\n' << '\n';

}
