/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ModuleBase.h
 *  @brief Base class for high-level Module types in MABE (Populations, Environments, etc.)
 *
 *  Note that organism types are passed into MABE, but only populuations of them are named.
 */

#ifndef MABE_MODULE_BASE_H
#define MABE_MODULE_BASE_H

#include <string>

namespace mabe {

  class ModuleBase {
  private:
    std::string name;

  public:
    const std::string & GetName() const { return name; }
    void SetName(std::string _in) { name = _in; }
  };

}

#endif

