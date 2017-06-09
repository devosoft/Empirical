//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include "../meta/IntPack.h"

#include "WorldModule.h"
#include "WorldModule_Select.h"

namespace emp {

  // An interface to setup the proper class heirarchy for World objects
  template <typename ORG, typename MOD_PACK> class World_Interface;   // Generic interface stub

  template <typename ORG, int... IMODS>
  class World_Interface<ORG, IntPack<IMODS...>> : public WorldModule<ORG, (emp::evo) IMODS...> {
    using parent_t = WorldModule<ORG, (emp::evo) IMODS...>;
  };

  // The outer-most level of World objects...

  template <typename ORG, emp::evo... MODS>
  class World : public World_Interface<ORG, pack::RUsort<IntPack<(int) MODS...>>> {
  private:
    using parent_t = WorldModule<ORG, MODS...>;
    using typename parent_t::fit_fun_t;
    using parent_t::random_ptr;
    using parent_t::AddOrg;
    using parent_t::AddOrgAt;

  public:
    World() { ; }

    // Insert an organism using the default insertion scheme.
    void Insert(const ORG & mem, size_t copy_count=1);

    // Inset and organism at a specific position.
    void InsertAt(const ORG & mem, const size_t pos);

    // Insert a random organism (constructor must facilitate!)
    template <typename... ARGS> void InsertRandomOrg(ARGS &&... args);

    // Insert a newborn by default rules, with parent information.
    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count=1);

    // If InsertBirth is provided with a fitness function, immediately calculate fitness of new org.
    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count,
                     const fit_fun_t & fit_fun);
  };

  template <typename ORG, emp::evo... MODS>
  void World<ORG, MODS...>::Insert(const ORG & mem, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      //const size_t pos =
      AddOrg(new_org);
      //SetupOrg(*new_org, &callbacks, pos);
    }
  }

  template <typename ORG, emp::evo... MODS>
  void World<ORG, MODS...>::InsertAt(const ORG & mem, const size_t pos) {
    Ptr<ORG> new_org = NewPtr<ORG>(mem);
    AddOrgAt(new_org, pos);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, emp::evo... MODS>
  template <typename... ARGS>
  void World<ORG, MODS...>::InsertRandomOrg(ARGS &&... args) {
    emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
    Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
    // const size_t pos =
    AddOrg(new_org);
    // SetupOrg(*new_org, &callbacks, pos);
  }

  template <typename ORG, emp::evo... MODS>
  void World<ORG, MODS...>::InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      AddOrgBirth(new_org, parent_pos);
      // SetupOrg(*new_org, &callbacks, pos);
    }
  }

  // If InsertBirth is provided with a fitness function, use it to calculate fitness of new org.
  template <typename ORG, emp::evo... MODS>
  void World<ORG, MODS...>::InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count,
                                        const fit_fun_t & fit_fun) {
    for (size_t i = 0; i < copy_count; i++) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      // const size_t pos =
      AddOrgBirth(new_org, parent_pos);
      // If we offspring are placed into the same population, recalculate fitness.
      // CalcFitness(pos, fit_fun);
      // SetupOrg(*new_org, &callbacks, pos);
    }
  }
}

#endif
