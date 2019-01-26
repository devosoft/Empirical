/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismTypeBase.h
 *  @brief Base class for all Organsim Types in MABE; describes how organism functions.
 *
 *  This file details all of the basic functionality that all organisms MUST have, providing
 *  reasonable defaults when such are possible.
 * 
 *  Every organism type must define an internal type 'Organism' that determines the type of each
 *  individual org.
 */

#ifndef MABE_ORGANISM_TYPE_BASE_H
#define MABE_ORGANISM_TYPE_BASE_H

#include <map>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/GenericFunction.h"

#include "ModuleBase.h"

namespace mabe {

  class OrganismTypeBase : public ModuleBase {
  protected:
    /// These are functions that were originally provided by the environment and wrapped by
    /// this organism type so that it will take an OrganismBase reference as its only argument
    /// and return a double.  The environment will call these functions when specific events
    /// are triggered.  Anything more complex than a double should be handled with a callback
    /// using one of the action functions in the next group.
    using event_fun_t = std::function<double(OrganismBase &)>;
    emp::vector<event_fun_t> event_funs;

    /// These are functions that were originally provided by the environment and wrapped by
    /// this organism type to be callable with a common interface (providing a reference to the
    /// calling organism).  The only return type allowed is a double; anything more complex
    /// should be handed with a callback.
    emp::vector<event_fun_t> action_funs;

  public:
    OrganismTypeBase(const std::string & in_name) : ModuleBase(in_name) { ; }    
    virtual ~OrganismTypeBase() {
    }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ORGANISM_TYPE; }

    /// Add a new event function for this organism type; wrap the function and store it.
    virtual bool AddEventFunction(FunctionInfo & info) = 0;

    /// Add a new action function for this organism type; wrap the function and store it.
    virtual bool AddActionFunction(FunctionInfo & info) = 0;

  };

}

#endif

