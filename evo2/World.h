//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include "WorldModule.h"

namespace emp {


  // The outer-most level of World objects...

  template <typename ORG, emp::evo... MODS>
  class World : public WorldModule<ORG, MODS...> {
  private:
    using parent_t = WorldModule<ORG, MODS...>;

  public:
  };

}

#endif
