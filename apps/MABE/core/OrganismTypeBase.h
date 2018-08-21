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
 *  All organisms must be able to deal with two types of functors:
 *   ACTIONS are functions that organisms can trigger through their execution or outputs.
 *   EVENTS are functions that environments will call to indicate input sent to the organisms.
 * 
 *  Every derived organism type must:
 *   1. Define an internal type 'Organism' that determines the type of each individual org.
 *   2. Implement set of AddActionFunction() member functions that provide functors that this
 *      organism type can use (or be asked to use), with a name, unique ID, type (as string),
 *      and description.  Return is a bool indicating success at using.
 *   3. Implement set of AddEventFunction() member functions that build GenericFunctions to
 *      call from the environment (with the appropriate args) when an event occurs.
 * 
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
    /// These are functions that were provided by the environment and wrapped by this organism
    /// type.  It will take an OrganismBase reference along with any event-specifc arguments.
    /// The environment will call these functions when associated specific events are triggered.
    using fun_ptr_t = emp::Ptr<emp::GenericFunction>;
    emp::vector<fun_ptr_t> action_funs;
    emp::vector<fun_ptr_t> event_funs;

  public:
    OrganismTypeBase(const std::string & in_name) : ModuleBase(in_name) { ; }    
    virtual ~OrganismTypeBase() {
    }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ORGANISM_TYPE; }

    template <typename... Ts>
    double TriggerEvent(OrganismBase & org, size_t event_id, Ts &&... args) {
      return event_funs[event_id]->Call<double, Ts...>(org, std::forward<Ts>(args)...);
    }

  };

}

#endif

