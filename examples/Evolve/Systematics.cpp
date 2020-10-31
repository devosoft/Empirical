//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the grid options for emp::World.h

#include <iostream>

#include "emp/Evolve/Systematics.hpp"

int main()
{
  emp::Systematics<int> sys(true, true, true);

  std::cout << "\nAddOrg 25 (id1, no parent)\n";
  auto id1 = sys.AddOrg(25);
  std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  auto id2 = sys.AddOrg(-10, id1);
  std::cout << "\nAddOrg 25 (id3; parent id1)\n";
  auto id3 = sys.AddOrg(25, id1);
  std::cout << "\nAddOrg 25 (id4; parent id2)\n";
  auto id4 = sys.AddOrg(25, id2);
  std::cout << "\nRemoveOrg (id2)\n";
  sys.RemoveOrg(id2);
  std::cout << "\nRemoveOrg (id4)\n";
  sys.RemoveOrg(id4);

  std::cout << "id1 = " << id1 << std::endl;
  std::cout << "id2 = " << id2 << std::endl;
  std::cout << "id3 = " << id3 << std::endl;
  std::cout << "id4 = " << id4 << std::endl;

  std::cout << "\nLineage:\n";
  sys.PrintLineage(id4);
  sys.PrintStatus();
}
