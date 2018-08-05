/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  WatcherBase.h
 *  @brief Base class for all watchers in MABE
 *
 *  This file details all of the basic functionality that all watchers MUST have, providing
 *  reasonable defaults when such are possible.  Watchers are tools that monitor an evolving
 *  population (collecting data or performing analyses), but do not alter its progress.
 */

#ifndef MABE_WATCHER_BASE_H
#define MABE_WATCHER_BASE_H

#include "ModuleBase.h"

namespace mabe {

  class WatcherBase : public ModuleBase {
  private:
  public:
    WatcherBase(const std::string & in_name) : ModuleBase(in_name) { ; }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::WATCHER; }
  };

}

#endif

