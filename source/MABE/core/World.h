/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World.h
 *  @brief A world management class, putting all MABE components together.
 *
 *  This templated object takes a set of MABE components and builds a working world from
 *  them, automatically setting up a configurtion system and using reasonable default
 *  linkages (that can be easily revised).
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include <string>

#include "../../base/vector.h"
#include "../../meta/TypePack.h"

namespace mabe {

  template <typename... MODULES>
  class World {
  public:
    using modules_t = emp::TypePack<MODULES...>;

  private:
    emp::vector<std::string> module_names;

  public:
    World() { ; }
  };

}

#endif

