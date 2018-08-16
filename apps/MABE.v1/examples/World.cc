/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World.cc
 *  @brief An example for using a MABE World.
 */

#include "../core/World.h"

class MyOrganism : public mabe::OrganismBase {
  int x;
};

int main(int argc, char *argvp[])
{
  mabe::World<MyOrganism, int, mabe::OrganismBase, std::string>
    world({"my_org", "int", "base_org", "string"});
}
