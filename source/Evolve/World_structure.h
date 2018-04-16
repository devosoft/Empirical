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
#include "../data/Trait.h"
#include "../tools/Random.h"
#include "../tools/vector_utils.h"

namespace emp {

  template <typename ORG> class World;

  /// Set the population to be a set of pools that are individually well mixed, but with limited
  /// migtation.  Arguments are the number of pools, the size of each pool, and whether the
  /// generations should be synchronous (true) or not (false, default).
  template <typename ORG>
  void SetPools(World<ORG> & world, size_t num_pools,
                size_t pool_size, bool synchronous_gen=false) {
    world.Resize(pool_size, num_pools);
    world.MarkSynchronous(synchronous_gen);
    world.MarkSpaceStructured(true).MarkPhenoStructured(false);

    // -- Setup functions --
    // Inject in an empty pool -or- randomly if none empty
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
      // Asynchronous: always go to a neighbor in current population.
      world.SetAddBirthFun( [&world](Ptr<ORG> new_org, size_t parent_id) {
        auto id = world.GetRandomNeighborID(parent_id);
        auto p_genotype = world.GetGenotypeAt(parent_id);
        return world.AddOrgAt(new_org, id, p_genotype); // Place org in existing population.
      });
      world.SetAttribute("SynchronousGen", "False");
    }

    world.SetAttribute("PopStruct", "Pools");
  }


  /// Set the population to use a MapElites structure.  This means that organism placement has
  /// two key components:
  /// 1: Organism position is based on their phenotypic traits.
  /// 2: Organisms must have a higher fitness than the current resident of a position to steal it.
  ///
  /// Note: Since organisms compete with their predecessors for space in the populations,
  /// synchronous generations do not make sense.
  ///
  /// This for version will setup a MAP-Elites world; traits to use an how many bins for each
  /// (trait counts) must be provided.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, TraitSet<ORG> traits,
                    const emp::vector<size_t> & trait_counts) {
    using org_pos_t = typename World<ORG>::OrgPosition;

    world.Resize(trait_counts);  // World sizes are based on counts of traits options.
    world.MarkSynchronous(false);
    world.MarkSpaceStructured(false).MarkPhenoStructured(true);

    // -- Setup functions --
    // Inject into the appropriate positon based on phenotype.  Note that an inject will fail
    // if a more fit organism is already in place; you must run clear first if you want to
    // ensure placement.
    world.SetAddInjectFun( [&world,traits,trait_counts](Ptr<ORG> new_org) {
      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);
      size_t id = traits.EvalBin(*new_org, trait_counts);
      double cur_fitness = world.CalcFitnessID(id);

      if (cur_fitness > org_fitness) return org_pos_t();  // Return invalid position!
      return world.AddOrgAt(new_org, id);
    });

    // Map Elites does not have a concept of neighbors.
    world.SetGetNeighborFun( [](size_t id) { emp_assert(false); return id; });

    // Birth is effectively the same as inject.
    world.SetAddBirthFun( [&world,traits,trait_counts](Ptr<ORG> new_org, size_t parent_id) {
      (void) parent_id; // Parent id is not needed for MAP Elites.
      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);
      size_t id = traits.EvalBin(*new_org, trait_counts);
      double cur_fitness = world.CalcFitnessID(id);

      if (cur_fitness > org_fitness) return org_pos_t();  // Return invalid position!
      return world.AddOrgAt(new_org, id);
    });

    world.SetAttribute("SynchronousGen", "False");
    world.SetAttribute("PopStruct", "MapElites");
  }

  /// Setup a MAP-Elites world, given the provided set of traits.
  /// Requires world to already have a size; that size is respected when deciding trait bins.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, TraitSet<ORG> traits) {
    emp::vector<size_t> trait_counts;
    emp_assert(traits.GetSize() > 0);

    // If there's only a single trait, it should get the full population.
    if (traits.GetSize() == 1) {
      trait_counts.push_back(world.GetSize());
      SetMapElites(world, traits, trait_counts);
      return;
    }
    const size_t num_traits = traits.GetSize();
    size_t trait_size = 1;
    while (Pow(trait_size+1, num_traits) < world.GetSize()) trait_size++;
    trait_counts.resize(num_traits, trait_size);
    SetMapElites(world, traits, trait_counts);
  }

  /// Setup a MAP-Elites world, given the provided trait counts (number of bins).
  /// Requires world to already have a phenotypes that those counts are applied to.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, const emp::vector<size_t> & trait_counts) {
    SetMapElites(world, world.GetPhenotypes(), trait_counts);
  }

  /// Setup a MAP-Elites world, given the provided worlds already has size AND set of phenotypes.
  /// Requires world to already have a size; that size is respected when deciding trait bins.
  template <typename ORG>
  void SetMapElites(World<ORG> & world) { SetMapElites(world, world.GetPhenotypes()); }



  /// DiverseElites is similar to MAP-Elites, but rather than merely keep the elites on
  /// a pre-defined grid, it merely tries to maintain maximal distance between elites in
  /// trait space.  The main advantages to this technique are (1) It's easy to build
  /// up an inital population that grows in diversity over time, and (2) You don't need to
  /// predefine box sizes or even limits to trait values.

  /// Set the population to use a DiverseElites structure.  This means that organism placement has
  /// two key components:
  /// 1: Organism position is in continuous space based on phenotypic traits.
  /// 2: When the population is full, nearby organisms must battle to keep their position.
  ///
  /// Note: Since organisms compete with their predecessors for space in the populations,
  /// synchronous generations do not make sense.

  /// This first version will setup a Diverse-Elites world and specify traits to use.
  template <typename ORG>
  void SetDiverseElites(World<ORG> & world, TraitSet<ORG> traits, size_t world_size) { 
    using org_pos_t = typename World<ORG>::OrgPosition;

    world.Resize(world_size);
    world.MarkSynchronous(false);
    world.MarkSpaceStructured(false).MarkPhenoStructured(true);

    // -- Setup functions --
    // Inject into the appropriate positon based on phenotype.  Note that an inject will fail
    // if a more fit organism is already in place; you must run clear first if you want to
    // ensure placement.
    world.SetAddInjectFun( [&world,traits](Ptr<ORG> new_org) {
      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);

      // @CAO Find empty spot?  Or compete nearby orgs to clear out a spot?
      return org_pos_t();  // @CAO For now, Return invalid position!
    });

    // Map Elites does not have a concept of neighbors.
    world.SetGetNeighborFun( [](size_t id) { emp_assert(false); return id; });

    // Birth is effectively the same as inject.
    world.SetAddBirthFun( [&world,traits](Ptr<ORG> new_org, size_t parent_id) {
      (void) parent_id; // Parent id is not needed for MAP Elites.

      // Determine tha position that this phenotype fits in.
      double org_fitness = world.CalcFitnessOrg(*new_org);

      // @CAO Find empty spot?  Or compete nearby orgs to clear out a spot?
      return org_pos_t();  // @CAO For now, Return invalid position!
    });

    world.SetAttribute("SynchronousGen", "False");
    world.SetAttribute("PopStruct", "DiverseElites");
  }

  /// Setup a MAP-Elites world, given the provided worlds already has set of phenotypes.
  template <typename ORG>
  void SetMapElites(World<ORG> & world, size_t world_size) {
    SetMapElites(world, world.GetPhenotypes(), world_size);
  }
}

#endif
