/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  EnvironmentBase.h
 *  @brief Base class for all Environments in MABE
 *
 *  This file details all of the basic functionality that all environments MUST have, providing
 *  reasonable defaults when such are possible.  Environments can describe the surrounding
 *  world that organisms can interact with -or- be a fitness function for use in an evolutionary
 *  algorithm.
 */

#ifndef MABE_ENVIRONMENT_BASE_H
#define MABE_ENVIRONMENT_BASE_H

#include "ModuleBase.h"

namespace mabe {

  class EnvironmentBase : public ModuleBase {
  private:
  public:
    EnvironmentBase(const std::string & in_name) : ModuleBase(in_name) { ; }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ENVIRONMENT; }
  };

}

#endif

