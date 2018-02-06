/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  World_structure.h
 *  @brief Functions for popular world structure methods.
 */

#ifndef EMP_EVO_WORLD_STRUCTURE_H
#define EMP_EVO_WORLD_STRUCTURE_H

#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/Random.h"

namespace emp {

  template<typename ORG> class World;

  /// Set the population to be a set of pools that are individually well mixed, but with limited
  /// migtation.  Arguments are the number of pools, the size of each pool, and whether the
  /// generations should be synchronous (true) or not (false, default).
  template<typename ORG>
  void SetPools(World<ORG> & world, size_t num_pools,
                size_t pool_size, bool synchronous_gen=false) {
    world.Resize(pool_size, num_pools);
    world.MarkSynchronous(synchronous_gen);
    world.MarkSpaceStructured(true).MarkPhenoStructured(false);

    // -- Setup functions --
    // Inject in a empty pool -or- randomly if none empty
    world.SetAddInjectFun( [&world,pool_size](Ptr<ORG> new_org) {
      for (size_t id = 0; id < world.GetSize(); id += pool_size) {
        if (world.IsOccupied(id) == false) return world.AddOrgAt(new_org, id);
      }
      return world.AddOrgAt(new_org, world.GetRandomCellID());
    });

    // Neighbors are everyone in the same pool.
    world.SetGetNeighborFun( [&world,pool_size](size_t id) {
      const size_t pool_start = (id / pool_size) * pool_size;
      return pool_start + world.GetRandom().GetUInt(pool_size);
    });

    if (synchronous_gen) {
      // Place births in the next open spot in the new pool (or randomly if full!)
      world.SetAddBirthFun( [&world,pool_size](Ptr<ORG> new_org, size_t parent_id) {
        emp_assert(new_org);                                  // New organism must exist.
        const size_t pool_id = parent_id / pool_size;
        const size_t start_id = pool_id * pool_size;
        for (size_t id = start_id; id < start_id+pool_size; id++) {
          if (world.IsOccupied(id) == false) {  // Search for an open positions...
            return world.AddNextOrgAt(new_org, id, world.GetGenotypeAt(parent_id));
          }
        }
        const size_t id = world.GetRandomNeighborID(parent_id);     // Placed near parent, in next pop.
        return world.AddNextOrgAt(new_org, id, world.GetGenotypeAt(parent_id));
      });
      world.SetAttribute("SynchronousGen", "True");
    } else {
      // Asynchronous: always go to a neigbor in current population.
      world.SetAddBirthFun( [&world](Ptr<ORG> new_org, size_t parent_id) {
        auto id = world.GetRandomNeighborID(parent_id);
        auto p_genotype = world.GetGenotypeAt(parent_id);
        return world.AddOrgAt(new_org, id, p_genotype); // Place org in existing population.
      });
      world.SetAttribute("SynchronousGen", "False");
    }

    world.SetAttribute("PopStruct", "Pools");
  }


  // template<typename ORG>
  // void EliteSelect(World<ORG> & world, size_t e_count=1, size_t copy_count=1) {
  //   emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
  //   emp_assert(copy_count > 0);

  //   // Load the population into a multimap, sorted by fitness.
  //   std::multimap<double, size_t> fit_map;
  //   for (size_t id = 0; id < world.GetSize(); id++) {
  //     if (world.IsOccupied(id)) {
  //       const double cur_fit = world.CalcFitnessID(id);
  //       fit_map.insert( std::make_pair(cur_fit, id) );
  //     }
  //   }

  //   // Grab the top fitnesses and move them into the next generation.
  //   auto m = fit_map.rbegin();
  //   for (size_t i = 0; i < e_count; i++) {
  //     const size_t repro_id = m->second;
  //     world.DoBirth( world.GetGenomeAt(repro_id), repro_id, copy_count);
  //     ++m;
  //   }
  // }

}

#endif
