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

#include "../../base/vector.h"
#include "../../meta/TypePack.h"
#include "../../tools/tuple_utils.h"

#include "types.h"

namespace mabe {

  class World {
  private:
    emp::vector<emp::Ptr<EnvironmentBase>> environments;
    emp::vector<emp::Ptr<PopulationBase>> populations;
    emp::vector<emp::Ptr<SchemaBase>> schemas;
    emp::vector<emp::Ptr<WatcherBase>> watchers;

    void AddModule(emp::Ptr<EnvironmentBase> env_ptr) { environment_ptrs.push_back(env_ptr); }
    void AddModule(emp::Ptr<ListenerBase> lis_ptr) { listener_ptrs.push_back(lis_ptr); }
    void AddModule(emp::Ptr<PopulationBase> pop_ptr) { population_ptrs.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schema_ptrs.push_back(schema_ptr); }

    void ForEachModule(emp::function<void(emp::Ptr<ModuleBase>)> fun) {
      foreach (Ptr<Module> x : environments) { fun(x); }
      foreach (Ptr<Module> x : populations)  { fun(x); }
      foreach (Ptr<Module> x : schemas)      { fun(x); }
      foreach (Ptr<Module> x : watchers)     { fun(x); }
    }

  public:
    World(emp::vector<std::string> names) {
    }

    ~World() {
      ForEachModule( [](emp::Ptr<ModuleBase> x){ x.Delete(); } );
    }

  };

}

#endif

