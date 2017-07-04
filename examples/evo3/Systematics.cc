//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the grid options for emp::World.h

#include <iostream>

#include "../../evo3/Systematics.h"

int main()
{
  emp::Systematics<int> sys(true, true, true);

  std::cout << "\nInject 25 (id1)\n";
  auto id1 = sys.InjectOrg(25);
  std::cout << "\nAddOrg -10 (id2; parent id1)\n";
  auto id2 = sys.AddOrg(id1, -10);
  std::cout << "\nAddOrg 25 (id3; parent id1)\n";
  auto id3 = sys.AddOrg(id1, 25);
  std::cout << "\nAddOrg 25 (id4; parent id2)\n";
  auto id4 = sys.AddOrg(id2, 25);
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
