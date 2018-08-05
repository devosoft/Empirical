/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ResourceGrid.h
 *  @brief A grid-based world where agents must collect pools of resources.
 */

#ifndef MABE_RESOURCE_GRID_H
#define MABE_RESOURCE_GRID_H

#include "core/EnvironmentBase.h"

namespace mabe {

  class ResourceGrid : public EnvironmentBase {
  private:
  public:
    ResourceGrid(const std::string & name) : EnvironmentBase(name) { ; }

    std::string GetClassName() const override { return "ResourceGrid"; }
  };

}

#endif

