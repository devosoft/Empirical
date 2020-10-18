/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  SchemaBase.h
 *  @brief Base class for all Update Schemas in MABE
 *
 *  This file details all of the basic functionality that all update schemas MUST have,
 *  providing reasonable defaults when such are possible.  Update schemas provide details
 *  for how a population should be run -- selection techniques (or other optimizers),
 *  events, or merely execution of organisms.
 *  
 */

#ifndef MABE_UPDATE_SCHEMA_BASE_H
#define MABE_UPDATE_SCHEMA_BASE_H

#include "ModuleBase.h"

namespace mabe {

  class SchemaBase : public ModuleBase {
  private:
  public:
    SchemaBase(const std::string & in_name) : ModuleBase(in_name) { ; }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::SCHEMA; }
  };

}

#endif

