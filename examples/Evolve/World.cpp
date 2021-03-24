//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file is an example for using the re-vamped World template.

#include <iostream>

#include "emp/Evolve/World.hpp"
#include "emp/math/Random.hpp"

struct TestOrg1 {
  int fitness;

  TestOrg1() : fitness(0) { ; }
  TestOrg1(int f) : fitness(f) { ; }
  double GetFitness() const { return (double) fitness; }
  bool DoMutate(emp::Random&) { return false; }

  bool operator==(const TestOrg1 & in) const { return fitness == in.fitness; }
  bool operator!=(const TestOrg1 & in) const { return fitness != in.fitness; }
};

int main() {
  emp::World<int> world;
  world.SetPopStruct_Mixed(true);

  world.SetFitFun([](int & i){ return (double) i; });
  for (int i = 0; i < 100; i++) {
    world.Inject(i+100,2);
  }

  std::cout << "Start Size = " << world.GetSize() << '\n' << '\n';
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << '\n';

  EliteSelect(world, 10, 10);

  std::cout << "\nElite Select(10,10)\n" << '\n';
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << '\n';

  TournamentSelect(world, 5, 100);
  world.Update();
  std::cout << "\nPost-Tourney Size = " << world.GetSize() << '\n' << '\n';
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << '\n';

  EliteSelect(world, 10, 10);
  world.Update();
  std::cout << "\nPost-Elite Size = " << world.GetSize() << '\n' << '\n';
  for (size_t i = 0; i < world.GetSize(); i++) std::cout << world[i] << " ";
  std::cout << '\n' << '\n';


  emp::World<TestOrg1> ea_world;
  ea_world.SetPopStruct_Mixed(true);
  for (int i = 0; i < 100; i++) ea_world.Inject(i+200);

  std::cout << "\nStart Size = " << ea_world.GetSize() << '\n';
  for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].GetFitness() << " ";
  std::cout << '\n';

  TournamentSelect(ea_world, 5, 100);
  ea_world.Update();
  std::cout << "\nPost-Tourney Size = " << ea_world.GetSize() << '\n';
  for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].GetFitness() << " ";
  std::cout << '\n';

  EliteSelect(ea_world, 10, 10);
  ea_world.Update();
  std::cout << "Post-Elite Size = " << ea_world.GetSize() << '\n';
  for (size_t i = 0; i < ea_world.GetSize(); i++) std::cout << ea_world[i].GetFitness() << " ";
  std::cout << '\n' << '\n';

  // Test grid Populations
  emp::Random random;
  emp::World<int> grid_world(random);
  grid_world.SetPopStruct_Grid(10,10);
  for (int i = 0; i < 10; i++) grid_world.Inject(i);
  grid_world.PrintGrid();

  for (size_t i = 0; i < grid_world.GetSize(); ++i) {
    size_t id = random.GetUInt(grid_world.GetSize());
    if (grid_world.IsOccupied(id)) grid_world.DoBirth(grid_world[id], id);
  }
  std::cout << '\n';
  grid_world.PrintGrid();
  std::cout << "Num orgs=" << grid_world.GetNumOrgs() << '\n';
  std::cout << '\n';
}
