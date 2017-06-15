//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file is an example for using the re-vamped World template.

#include <iostream>

#include "../../evo3/World.h"
#include "../../tools/Random.h"

struct TestOrg1 {
  int fitness;

  TestOrg1() : fitness(0) { ; }
  TestOrg1(int f) : fitness(f) { ; }
  double Fitness() { return (double) fitness; }
  bool Mutate(emp::Random&) { return false; }
};

int main() {
  emp::World<int> world;
  world.ModeEA();

  world.SetFitFun([](int & i){ return (double) i; });
  for (int i = 0; i < 100; i++) {
    world.Inject(i+100,2);
  }

  std::cout << "Start Size = " << world.GetSize() << std::endl << std::endl;
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  world.EliteSelect(10, 10);

  std::cout << "\nElite Select(10,10)\n" << std::endl;
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  world.TournamentSelect(5, 100);
  world.Update();
  std::cout << "\nPost-Tourney Size = " << world.GetSize() << std::endl << std::endl;
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  // // Populations can be supplied a default fitness function so that it doesn't
  // // need to keep being specified.
  // world.SetDefaultFitnessFun([](int * i){ return (double) *i; });
  //
  // // world.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  // world.EliteSelect(10, 10);
  // world.Update();
  // std::cout << "Post-Elite Size = " << world.GetSize() << std::endl;
  // for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  // std::cout << std::endl << std::endl;
  //
  //
  //
  // EAWorld<TestOrg1> ea_world(random);
  // for (int i = 0; i < 100; i++) ea_world.Inject(i+200);
  //
  // std::cout << "Start Size = " << ea_world.GetSize() << std::endl;
  // for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  // std::cout << std::endl;
  //
  // ea_world.TournamentSelect(5, 100);
  // ea_world.Update();
  // std::cout << "Post-Tourney Size = " << ea_world.GetSize() << std::endl;
  // for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  // std::cout << std::endl;
  //
  // // world.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  // ea_world.EliteSelect(10, 10);
  // ea_world.Update();
  // std::cout << "Post-Elite Size = " << ea_world.GetSize() << std::endl;
  // for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  // std::cout << std::endl << std::endl;
  //
  //
  // // Test grid Populations
  // GridWorld<int> grid_world(random);
  // for (int i = 0; i < 10; i++) grid_world.Inject(i);
  // grid_world.Print();
  //
  // for (size_t i = 0; i < grid_world.GetSize(); ++i) {
  //   size_t id = random.GetUInt(grid_world.GetSize());
  //   if (grid_world.IsOccupied(id)) grid_world.InjectBirth(grid_world[id], id);
  // }
  // std::cout << std::endl;
  // grid_world.Print();
  // std::cout << "Num orgs=" << grid_world.GetNumOrgs() << std::endl;
  // std::cout << std::endl;
  //
  //
  // // Let's try to build a world with a plug-in population manager.
  // // We will have each organism replaced on birth be on the left side of parent.
  // World<int, PopPlugin> pi_world(random, "pi_world");
  //
  // pi_world.popM.OnClear( [&pi_world]() {
  //   pi_world.popM.Resize(0);
  //   for (int i=0;i<20;i++) pi_world.Inject(i+100);
  // });
  //
  // pi_world.popM.OnAddOrg( [&pi_world](int* org, size_t& pos) {
  //   pos = pi_world.popM.AddOrgAppend(org);
  // });
  //
  // pi_world.popM.OnAddOrgBirth( [&pi_world](int* org, size_t parent_pos, size_t& pos) {
  //   if (parent_pos == 0) pos = pi_world.popM.size() - 1;
  //   else pos=parent_pos-1;
  //   pi_world.popM.AddOrgAt(org, pos);
  // });
  //
  // pi_world.Clear();
  // std::cout << "Starting PIWorld size: " << pi_world.GetSize() << std::endl;
  // for (size_t i = 0; i < pi_world.GetSize(); i++) std::cout << pi_world[i] << " ";
  // std::cout << std::endl;
  //
  // pi_world.TournamentSelect([](int * i){ return (double) *i; }, 2, 100);
  // std::cout << "Post-Tourney Size = " << pi_world.GetSize() << std::endl;
  // for (size_t i = 0; i < pi_world.GetSize(); i++) std::cout << pi_world[i] << " ";
  // std::cout << std::endl;
}
