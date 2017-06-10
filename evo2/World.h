//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include "../meta/IntPack.h"

#include "WorldModule.h"
#include "WorldModule_Select.h"

namespace emp {

  // An interface to setup the proper class heirarchy for World objects
  template <typename ORG, typename MOD_PACK> class World_Interface;   // Generic interface stub

  template <typename ORG, int... IMODS>
  class World_Interface<ORG, IntPack<IMODS...>> : public WorldModule<ORG, (emp::evo) IMODS...> {
    using parent_t = WorldModule<ORG, (emp::evo) IMODS...>;
  };

  // The outer-most level of World objects...

  template <typename ORG, emp::evo... MODS>
  class World : public World_Interface<ORG, pack::RUsort<IntPack<(int) MODS...>>> {
  private:
    using parent_t = WorldModule<ORG, MODS...>;
    using typename parent_t::fit_fun_t;
    using parent_t::random_ptr;
    using parent_t::AddOrg;
    using parent_t::AddOrgAt;

  public:
    World() { ; }
  };

}

#endif
