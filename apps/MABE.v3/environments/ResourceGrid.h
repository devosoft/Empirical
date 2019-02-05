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
    EMP_BUILD_CONFIG( RGConfig,
      GROUP(DEFAULT_GROUP, "Resource Grid Settings"),
      VALUE(FILENAME, std::string, "resource_grid.dat", "File containing resource grid layout"),
      VALUE(RESOURCE_MOD, double, 1, "Score modification when resources are consumed."),
      VALUE(EMPTY_MOD, double, -0.5, "Score modification when an empty square is accessed."),
      VALUE(USED_MOD, double, 0.0, "Score modification when a former resource cell is accessed."),
      VALUE(STEP_MOD, double, 0.0, "Cost of taking any step anywhere."),
    )  

    RGConfig config;
  public:
    ResourceGrid(const std::string & name) : EnvironmentBase(name) { ; }

    std::string GetClassName() const override { return "ResourceGrid"; }

    /// Required accessor for configuration objects.
    RGConfig & GetConfig() override { return config; }
  };

}

#endif

