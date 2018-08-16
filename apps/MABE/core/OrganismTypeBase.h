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
    using fun_ptr_t = emp::Ptr<emp::GenericFunction>;
    std::map< std::string, fun_ptr_t > event_fun_map;
    std::map< std::string, fun_ptr_t > action_fun_map;

  public:
    OrganismTypeBase(const std::string & in_name) : ModuleBase(in_name) { ; }    
    virtual ~OrganismTypeBase() {
      for (auto & x : event_fun_map) x.second.Delete();
      for (auto & x : action_fun_map) x.second.Delete();
    }

    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::ORGANISM_TYPE; }

    /// Event functions are provided by the derived OrganismType and called by the environment
    /// whenever there is an event, such as resources appearing, movement occuring, etc.  The
    /// environment must specify which organism is affected by the event, and any unique event
    /// information.
    template <typename... Ts>
    void AddEventFunction(const std::string & event_name, std::function<void(size_t, Ts...)> fun) {
      auto new_fun = emp::NewPtr< emp::Function<void(size_t, Ts...)> >(fun);
      event_fun_map[event_name] = new_fun;
    }

    /// Action functions are provided by the environment and allow organisms to take actions
    /// such as moving, sensing, etc.  The function call must specify the unique organism ID and
    /// any extra information needed by the environment.
    template <typename... Ts>
    void AddActionFunction(const std::string & action_name, std::function<void(size_t, Ts...)> fun) {
      auto new_fun = emp::NewPtr< emp::Function<void(size_t, Ts...)> >(fun);
      action_fun_map[action_name] = new_fun;
    }
  };

}

#endif

