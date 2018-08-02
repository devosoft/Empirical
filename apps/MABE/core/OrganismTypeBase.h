/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismBase.h
 *  @brief Base class for all Organsims in MABE
 *
 *  This file details all of the basic functionality that all organisms MUST have, providing
 *  reasonable defaults when such are possible.
 */

#ifndef MABE_ORGANISM_TYPE_BASE_H
#define MABE_ORGANISM_TYPE_BASE_H

#include "ModuleBase.h"

namespace mabe {

  class OrganismTypeBase {
  private:
  public:
    OrganismTypeBase(const std::string & in_name) : ModuleBase(in_name) { ; }    

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ORGANISM_TYPE; }
  };

}

#endif

