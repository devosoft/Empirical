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
#include "../../tools/tuple_utils.h"

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

    using environments_tup_t = typename environments_t::template apply<std::tuple>;
    using listeners_tup_t    = typename listeners_t::template apply<std::tuple>;
    using populations_tup_t  = typename populations_t::template apply<std::tuple>;
    using schemas_tup_t      = typename schemas_t::template apply<std::tuple>;

    environments_tup_t environments;  ///< Instances of environment modules.
    listeners_tup_t listeners;        ///< Instances of listener modules. 
    populations_tup_t populations;    ///< Instances of population modules.
    schemas_tup_t schemas;            ///< Instances of schema modules.

    /// Pointers to modules for easy iterartive access.
    emp::vector<emp::Ptr<ModuleBase>> module_ptrs;

    emp::vector<emp::Ptr<EnvironmentBase>> environment_ptrs;
    emp::vector<emp::Ptr<ListenerBase>> listener_ptrs;
    emp::vector<emp::Ptr<PopulationBase>> population_ptrs;
    emp::vector<emp::Ptr<SchemaBase>> schema_ptrs;


    void AddModule(emp::Ptr<EnvironmentBase> env_ptr) { environment_ptrs.push_back(env_ptr); }
    void AddModule(emp::Ptr<ListenerBase> lis_ptr) { listener_ptrs.push_back(lis_ptr); }
    void AddModule(emp::Ptr<PopulationBase> pop_ptr) { population_ptrs.push_back(pop_ptr); }
    void AddModule(emp::Ptr<SchemaBase> schema_ptr) { schema_ptrs.push_back(schema_ptr); }

    // template <typename T> void BuildModules() {
    //   emp::Ptr<T> new_ptr = emp::NewPtr<T>();
    //   AddModule(new_ptr);
    //   all_modules.push_back(new_ptr);
    // }
    // template <typename T1, typename T2, typename... Ts>
    // void BuildModules() {
    //   BuildModules<T1>();
    //   BuildModules<T2, Ts...>();
    // }

  public:
    World(emp::vector<std::string> names) {
      emp::TupleIterate(populations, [this](PopulationBase & pop) mutable { module_ptrs.push_back(&pop); });

      std::cout << "#modules = " << modules_t::GetSize() << std::endl;
      std::cout << "#environment types = " << environments_t::GetSize() << std::endl;
      std::cout << "#listener types    = " << listeners_t::GetSize() << std::endl;
      std::cout << "#organism types    = " << organisms_t::GetSize() << std::endl;
      std::cout << "#population types  = " << populations_t::GetSize() << std::endl;
      std::cout << "#schema types      = " << schemas_t::GetSize() << std::endl;

      std::cout << "\nModule Names:\n";
      for (const std::string & name : names) std::cout << "  " << name << std::endl;
    }

  };

}

#endif

