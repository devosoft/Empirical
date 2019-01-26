/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ModuleBase.h
 *  @brief Base class for high-level Module types in MABE (Environments, OrganismTypes, etc.)
 *
 *  This class provides a base class for ALL modules in MABE, specifying common functionality.
 * 
 *  The author of a new MABE module *must* override:
 *
 *    const std::string & GetClassName() const
 *      Provide a unique name for module class, usually the same as in code (used for debugging).
 * 
 *    emp::Config & GetConfig()
 *      Provide access to this modules configuation file needs.
 * 
 *  The author of a new MABE module *may* also choose to override:
 * 
 *    Setup(World &)
 *      As modules are created, they will be given the opportunity to modify world settings,
 *      module settings, or attach to world signals, as needed.
 *    
 */

#ifndef MABE_MODULE_BASE_H
#define MABE_MODULE_BASE_H

#include <string>
#include "config/config.h"

namespace mabe {

  /// Track the specific type associated with a module.
  enum class ModuleType {
    NONE=0, BASE, ENVIRONMENT, ORGANISM_TYPE, SCHEMA, UNKNOWN
  };

  /// Allow modules to know about the eventual existance for the world object.
  class World;

  /// Base class for all major MABE modules that can receive names.
  class ModuleBase {
  private:
    std::string name;

  public:
    ModuleBase(const std::string & in_name) : name(in_name) { ; }
    virtual ~ModuleBase() { ; }  ///< Make sure all modules can be deleted properly.

    /// Identify the broad type of this module at compile time; ModuleType is enumerated above.
    /// These should be taken care of in the base classes of modules and not needed in derived class
    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::BASE; }

    /// Every module must have a unique name to identify its section of config files.
    const std::string & GetName() const { return name; }

    /// Every module type needs to specify its derived class name as a string.
    virtual std::string GetClassName() const = 0;

    /// Every module type needs to be able to provide the config object that it's using.
    /// (and empty emp::Config can be returned if there really are no config parameters.)
    virtual emp::Config & GetConfig() = 0;

    /// After config is finalized, modules will be provided with a World object.  This function
    /// should be used to configure world, to configure the module, or link up signals.
    virtual void Setup(World &) { ; }

  };

  /// Details about a generic function being passed between OrganismTypes and Environments.
  struct FunctionInfo {
    using fun_ptr_t = emp::Ptr<emp::GenericFunction>;
    size_t id;          ///< Unique ID for this type of function.
    fun_ptr_t fun_ptr;  ///< Pointer to generic function to be called.
    std::string type;   ///< A string representation of the C++ type of this function.
    std::string name;   ///< Unique name for this function.
    std::string desc;   ///< Full description for what this function does.
  };

}

#endif

