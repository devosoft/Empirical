/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ListenerBase.h
 *  @brief Base class for all Listeners in MABE
 *
 *  This file details all of the basic functionality that all listeners MUST have, providing
 *  reasonable defaults when such are possible.  Listeners are tools that monitor an evolving
 *  population (collecting data or performing analyses), but do not alter its progress.
 */

#ifndef MABE_LISTENER_BASE_H
#define MABE_LISTENER_BASE_H

#include "ModuleBase.h"

namespace mabe {

  class ListenerBase : public ModuleBase {
  private:
  public:
    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::LISTENER; }
  };

}

#endif

