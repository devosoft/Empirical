/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World.h
 *  @brief A world management class, putting MABE modles together.
 *
 *  A world assembles a set of MABE modules into an evolving population.  It also
 *  automatically sets up a configurtion system and uses reasonable default linkages
 *  (that can be easily overridden).
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include <iostream>
#include <string>

#include "base/vector.h"
#include "meta/TypePack.h"
#include "tools/tuple_utils.h"

#include "types.h"

namespace mabe {

  class World {
  private:
    using environments_t = emp::vector<emp::Ptr<EnvironmentBase>>;
    using organism_types_t = emp::vector<emp::Ptr<OrganismTypeBase>>;
    using schemas_t = emp::vector<emp::Ptr<SchemaBase>>;
    using watchers_t = emp::vector<emp::Ptr<WatcherBase>>;

    using modules_t = std::tuple<environments_t, organism_types_t, schemas_t, watchers_t>;

    modules_t modules;    ///< Pointers to all modules, divided into module-type vectors.

    environments_t & environments;     ///< Direct link to environments vector of modules. 
    organism_types_t & organism_types; ///< Direct link to organism types vector of modules. 
    schemas_t & schemas;               ///< Direct link to schemas vector of modules. 
    watchers_t & watchers;             ///< Direct link to watchers vector of modules. 

    void AddModule(emp::Ptr<EnvironmentBase> env_ptr) { environments.push_back(env_ptr); }
    void AddModule(emp::Ptr<ListenerBase> lis_ptr) { listeners.push_back(lis_ptr); }
    void AddModule(emp::Ptr<OrganismTypeBase> pop_ptr) { organism_types.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schemas.push_back(schema_ptr); }

    void ForEachModule(emp::function<void(emp::Ptr<ModuleBase>)> fun) {
      foreach (Ptr<Module> x : environments) { fun(x); }
      foreach (Ptr<Module> x : organism_types)  { fun(x); }
      foreach (Ptr<Module> x : schemas)      { fun(x); }
      foreach (Ptr<Module> x : watchers)     { fun(x); }
    }

  public:
    World(emp::vector<std::string> names)
    : environments(std::get<environments_t>(modules))
    , organism_types(std::get<organism_types_t>(modules))
    , schemas(std::get<schemas_t>(modules))
    , watchers(std::get<watchers_t>(modules))
    {
    }

    ~World() {
      ForEachModule( [](emp::Ptr<ModuleBase> x){ x.Delete(); } );
    }

    template <typename T>
    T & BuildModule(const std::string name) {
      using module_t = to_module_t<T>;
      using mvector_t = emp::vector<emp::Ptr<module_t>>;
      emp::Ptr<module_t> new_mod = emp::NewPtr<T>(name)
      std::get<mvector_t>(modules).push_back(new_mod);
      return *new_mod;
    }

    void PrintStatus() {
      std::cout << "Environemnts: " << environments.GetSize() << std::endl;
      std::cout << "Organism Types: " << organism_types.GetSize() << std::endl;
      std::cout << "Schemas: " << schemas.GetSize() << std::endl;
      std::cout << "Watchers: " << watchers.GetSize() << std::endl;
    }
  };

}

#endif

