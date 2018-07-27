/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World.h
 *  @brief A world management class, putting all MABE components together.
 *
 *  This templated object takes a set of MABE components and builds a working world from
 *  them, automatically setting up a configurtion system and using reasonable default
 *  linkages (that can be easily revised).
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include <iostream>
#include <string>

#include "../../base/vector.h"
#include "../../meta/TypePack.h"

#include "types.h"

namespace mabe {

  template <typename... MODULES>
  class World {
  private:

    using modules_t = emp::TypePack<MODULES...>;
    using environments_t = typename modules_t::template filter<is_environment>;
    using listeners_t    = typename modules_t::template filter<is_listener>;
    using organisms_t    = typename modules_t::template filter<is_organism>;
    using populations_t  = typename organisms_t::template wrap<Population>;     ///< Org types determine pop types.
    using schemas_t      = typename modules_t::template filter<is_schema>;

    emp::vector<emp::Ptr<EnvironmentBase>> environments;
    emp::vector<emp::Ptr<ListenerBase>> listeners;
    emp::vector<emp::Ptr<PopulationBase>> populations;
    emp::vector<emp::Ptr<SchemaBase>> schemas;

    emp::vector<emp::Ptr<ModuleBase>> all_modules;
    emp::vector<std::string> module_names;

    void SetModuleNames() { ; }
    template <typename... Ts>
    void SetModuleNames(const std::string & name, Ts &&... extras...) {
      module_names.push_back(name);
      SetModuleNames(extras...);
    }

    void AddModule(emp::Ptr<EnvironmentBase> env_ptr) { environments.push_back(env_ptr); }
    void AddModule(emp::Ptr<ListenerBase> lis_ptr) { listeners.push_back(lis_ptr); }
    void AddModule(emp::Ptr<PopulationBase> pop_ptr) { populations.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schemas.push_back(schema_ptr); }

    template <typename T> void BuildModules() {
      emp::Ptr<T> new_ptr = emp::NewPtr<T>();
      AddModule(new_ptr);
      all_modules.push_back(new_ptr);
    }
    template <typename T1, typename T2, typename... Ts>
    void BuildModules() {
      BuildModules<T1>();
      BuildModules<T2, Ts...>();
    }

  public:
    template <typename... Ts>
    World(Ts... names) {
      SetModuleNames(names...);

      std::cout << "#modules = " << modules_t::GetSize() << std::endl;
      std::cout << "#environment types = " << environments_t::GetSize() << std::endl;
      std::cout << "#listener types    = " << listeners_t::GetSize() << std::endl;
      std::cout << "#organism types    = " << organisms_t::GetSize() << std::endl;
      std::cout << "#population types  = " << populations_t::GetSize() << std::endl;
      std::cout << "#schema types      = " << schemas_t::GetSize() << std::endl;

      std::cout << "\nModule Names:\n";
      for (const std::string & name : module_names) std::cout << "  " << name << std::endl;
    }

    ~World() {
      /// Delete all modules.
      for (auto & x : all_modules) x.Delete();
    }
  };

}

#endif

