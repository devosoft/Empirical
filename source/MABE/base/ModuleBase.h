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

  /// Track the specific type associated with a module.
  enum class ModuleType { NONE=0, BASE, ENVIRONMENT, LISTENER, ORGANISM, POPULATION, SCHEMA, UNKNOWN };

  /// Base class for all major MABE modules that can receive names.
  class ModuleBase {
  private:
    std::string name;

  public:
    virtual ~ModuleBase() { ; }  ///< Make sure all modules can be deleted properly.

    const std::string & GetName() const { return name; }
    void SetName(std::string _in) { name = _in; }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::BASE; }
  };

}

#endif

