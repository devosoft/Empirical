//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/World.h"
#include "../../tools/Random.h"

namespace evo = emp::evo;

struct TestOrg1 {
  int fitness;

  TestOrg1() { fitness = 0; }
  TestOrg1(int f) { fitness = f; }
  double Fitness() { return (double) fitness; }
  bool Mutate(emp::Random&) { return false; }
};

int main()
{
  emp::Random random;
  evo::World<int> world(random);

  for (int i = 0; i < 100; i++) world.Insert(i+100);

  std::cout << "Start Size = " << world.GetSize() << std::endl;
  for (int i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  world.TournamentSelect([](int * i){ return (double) *i; }, 5, 100);
  world.Update();
  std::cout << "Post-Tourney Size = " << world.GetSize() << std::endl;
  for (int i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl;

  // Populations can be supplied a default fitness function so that it doesn't
  // need to keep being specified.
  world.SetDefaultFitnessFun([](int * i){ return (double) *i; });

  // world.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  world.EliteSelect(10, 10);
  world.Update();
  std::cout << "Post-Elite Size = " << world.GetSize() << std::endl;
  for (int i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << std::endl << std::endl;



  evo::EAWorld<TestOrg1> ea_world(random);
  for (int i = 0; i < 100; i++) ea_world.Insert(i+200);

  std::cout << "Start Size = " << ea_world.GetSize() << std::endl;
  for (int i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  std::cout << std::endl;

  ea_world.TournamentSelect(5, 100);
  ea_world.Update();
  std::cout << "Post-Tourney Size = " << ea_world.GetSize() << std::endl;
  for (int i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  std::cout << std::endl;

  // world.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  ea_world.EliteSelect(10, 10);
  ea_world.Update();
  std::cout << "Post-Elite Size = " << ea_world.GetSize() << std::endl;
  for (int i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].Fitness() << " ";
  std::cout << std::endl << std::endl;


  // Test grid Populations
//  evo::World<int, evo::PopGrid> grid_world(random);
  evo::GridWorld<int> grid_world(random);
  for (int i = 0; i < 10; i++) grid_world.Insert(i);
  grid_world.Print();

  for (int i = 0; i < grid_world.GetSize(); ++i) {
    int id = random.GetInt(grid_world.GetSize());
    if (grid_world.IsOccupied(id)) grid_world.InsertBirth(grid_world[id], id);
  }
  std::cout << std::endl;
  grid_world.Print();
  std::cout << std::endl;


  // Let's try to build a world with a plug-in population manager.
  // We will have each organism replaced on birth be on the left side of parent.
  evo::World<int, evo::PopPlugin> pi_world(random, "pi_world");

  pi_world.popM.OnClear( [](emp::vector<int*>& pop) {
    for (auto x:pop) if(x) delete x;
    pop.resize(0);
    for (int i=0;i<20;i++) pop.push_back(new int (i+100));
  });

  pi_world.popM.OnAddOrg( [](emp::vector<int*>& pop, int* org, int& pos) {
    pos=pop.size(); pop.push_back(org);
  });

  pi_world.popM.OnAddOrgBirth( [](emp::vector<int*>& pop, int* org, int parent_pos, int& pos) {
    pos=parent_pos-1; if (pos < 0) pos = pop.size() - 1;
    if (pop[pos]) delete pop[pos];
    pop[pos] = org;
  });

  pi_world.Clear();
  std::cout << "Starting PIWorld size: " << pi_world.GetSize() << std::endl;
  for (int i = 0; i < pi_world.GetSize(); i++) std::cout << pi_world[i] << " ";
  std::cout << std::endl;

  pi_world.TournamentSelect([](int * i){ return (double) *i; }, 2, 100);
  std::cout << "Post-Tourney Size = " << pi_world.GetSize() << std::endl;
  for (int i = 0; i < pi_world.GetSize(); i++) std::cout << pi_world[i] << " ";
  std::cout << std::endl;
}
