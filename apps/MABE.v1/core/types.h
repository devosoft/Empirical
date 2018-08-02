/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  types.h
 *  @brief Manipulation tools to quickly identify MABE-specific types.
 */

#ifndef MABE_TYPES_H
#define MABE_TYPES_H

#include "../../base/vector.h"
#include "../../meta/TypePack.h"

#include "../base/EnvironmentBase.h"
#include "../base/ListenerBase.h"
#include "../base/OrganismBase.h"
#include "../base/SchemaBase.h"

#include "Population.h"

namespace mabe {
  template <typename T> using is_module      = std::is_base_of<ModuleBase, T>;
  template <typename T> using is_environment = std::is_base_of<EnvironmentBase, T>;
  template <typename T> using is_listener    = std::is_base_of<ListenerBase, T>;
  template <typename T> using is_organism    = std::is_base_of<OrganismBase, T>;
  template <typename T> using is_population  = std::is_base_of<PopulationBase, T>;
  template <typename T> using is_schema      = std::is_base_of<SchemaBase, T>;
}

#endif

