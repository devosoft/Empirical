//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h

#include <iostream>

#include "../../evo/Population.h"
#include "../../tools/Random.h"

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
  emp::evo::Population<int> pop(random);

  for (int i = 0; i < 100; i++) pop.Insert(i+100);

  std::cout << "Start Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl;

  pop.TournamentSelect([](int * i){ return (double) *i; }, 5, 100);
  pop.Update();
  std::cout << "Post-Tourney Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl;

  // Populations can be supplied a default fitness function so that it doesn't
  // need to keep being specified.
  pop.SetDefaultFitnessFun([](int * i){ return (double) *i; });

  // pop.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  pop.EliteSelect(10, 10);
  pop.Update();
  std::cout << "Post-Elite Size = " << pop.GetSize() << std::endl;
  for (int i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << " ";
  std::cout << std::endl << std::endl;



  emp::evo::Population<TestOrg1> pop1(random);
  for (int i = 0; i < 100; i++) pop1.Insert(i+200);

  std::cout << "Start Size = " << pop1.GetSize() << std::endl;
  for (int i = 0; i < pop1.GetSize(); i++) std::cout << pop1[i].Fitness() << " ";
  std::cout << std::endl;

  pop1.TournamentSelect(5, 100);
  pop1.Update();
  std::cout << "Post-Tourney Size = " << pop1.GetSize() << std::endl;
  for (int i = 0; i < pop1.GetSize(); i++) std::cout << pop1[i].Fitness() << " ";
  std::cout << std::endl;

  // pop.EliteSelect([](int * i){ return (double) *i; }, 10, 10);
  pop1.EliteSelect(10, 10);
  pop1.Update();
  std::cout << "Post-Elite Size = " << pop1.GetSize() << std::endl;
  for (int i = 0; i < pop1.GetSize(); i++) std::cout << pop1[i].Fitness() << " ";
  std::cout << std::endl << std::endl;
}
