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

#include "base/Ptr.h"
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
    void AddModule(emp::Ptr<WatcherBase> watch_ptr) { watchers.push_back(watch_ptr); }
    void AddModule(emp::Ptr<OrganismTypeBase> pop_ptr) { organism_types.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schemas.push_back(schema_ptr); }

    void ForEachModule(std::function<void(emp::Ptr<ModuleBase>)> fun) {
      for (emp::Ptr<ModuleBase> x : environments)    { fun(x); }
      for (emp::Ptr<ModuleBase> x : organism_types)  { fun(x); }
      for (emp::Ptr<ModuleBase> x : schemas)         { fun(x); }
      for (emp::Ptr<ModuleBase> x : watchers)        { fun(x); }
    }

  public:
    World()
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
      emp::Ptr<T> new_mod = emp::NewPtr<T>(name);
      std::get<mvector_t>(modules).push_back(new_mod);
      return *new_mod;
    }

    void Config(const std::string & filename, int argc, char * argv[]) {
      (void) filename;
      (void) argc;
      (void) argv;
      // @CAO Setup configurations here!
    }
    
    int Run() {
      return 0;
    }

    void PrintStatus() {
      std::cout << "Environemnts: " << environments.size() << std::endl;
      for (auto x : environments) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
      std::cout << "Organism Types: " << organism_types.size() << std::endl;
      for (auto x : organism_types) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
      std::cout << "Schemas: " << schemas.size() << std::endl;
      for (auto x : schemas) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
      std::cout << "Watchers: " << watchers.size() << std::endl;
      for (auto x : watchers) { 
        std::cout << "  " << x->GetName() << " (class name: " << x->GetClassName() << ")" << std::endl;
      }
    }
  };

}

#endif

