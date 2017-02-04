//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a base-class World for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_BASE_H
#define EMP_EVO_WORLD_BASE_H

#include <functional>
#include <map>

#include "../control/SignalControl.h"
#include "../meta/reflection.h"
#include "../tools/assert.h"
#include "../tools/Random.h"
#include "../tools/vector.h"

#include "FitnessManager.h"
#include "OrgSignals.h"
#include "OrgManager.h"
#include "PopulationManager.h"
#include "StatsManager.h"
#include "LineageTracker.h"


namespace emp {
namespace evo {

  EMP_SETUP_TYPE_SELECTOR(SelectFitnessManager, emp_is_fitness_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectPopManager, emp_is_population_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectOrgManager, emp_is_organism_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectStatsManager, emp_is_stats_manager)

  EMP_SETUP_TYPE_SELECTOR(SelectLineageManager, emp_is_lineage_manager)

  // The World managers pack organizes the various managers selected, fills in defaults, and
  // makes sure they are setup in the correct order.  All manages will be called with an
  // AdaptTemplate to provide the ORG type.

  template <typename ORG, typename... MANAGERS>
  using WorldManagers = TypePack<
    SelectStatsManager   <MANAGERS..., NullStats>,    // Stats manage has access to all others.
    SelectOrgManager     <MANAGERS..., OrgMDynamic>,  // Org manager can call back to population
    SelectPopManager     <MANAGERS..., PopBasic>,     // Pop manager can access fitness
    SelectFitnessManager <MANAGERS..., FitCacheOff>,
    World_Base                                        // All managers can access common base.
  >;

  // Given the org type and the WorldManager pack, return the next world manager, properly adapted.
  template <typename ORG, typename WM>
  using NextWorldManager = AdaptTemplate< WM::first, ORG, WM::pop >

  // Base class for all worlds (and managers)
  template <typename ORG, typename WM>
  class World_Base {
  protected:
    Random * random_ptr;
    bool random_owner;
    size_t update = 0;
    std::string world_name;

    World_Base() { ; }
    ~World_Base() { if (random_owner) delete random_ptr; }
  public:
    Random & GetRandom() { return *random_ptr; }
    void SetRandom(Random & random) { if (random_owner) delete random_ptr; random_ptr = &random; }
    void ResetRandom(int seed=-1) { SetRandom(*(new Random(seed))); }
  };


}  // END evo namespace
}  // END emp namespace

#endif
