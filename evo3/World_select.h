//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Handle selection methods for worlds.

#ifndef EMP_EVO_WORLD_SELECT_H
#define EMP_EVO_WORLD_SELECT_H

#include <map>

#include "../base/assert.h"
#include "../tools/IndexMap.h"

namespace emp {

  template<typename ORG> class World;

  /// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
  /// the next generation.  Find top e_count individuals and make copy_count copies of each.
  template<typename ORG>
  void EliteSelect(World<ORG> & world, size_t e_count, size_t copy_count) {
    emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
    emp_assert(copy_count > 0);

    // Load the population into a multimap, sorted by fitness.
    std::multimap<double, size_t> fit_map;
    for (size_t id = 0; id < world.GetSize(); id++) {
      if (world.IsOccupied(id)) {
        const double cur_fit = world.CalcFitnessID(id);
        fit_map.insert( std::make_pair(cur_fit, id) );
      }
    }

    // Grab the top fitnesses and move them into the next generation.
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < e_count; i++) {
      const size_t repro_id = m->second;
      world.DoBirth( world[repro_id], repro_id, copy_count);
      ++m;
    }
  }


  /// ==TOURNAMENT== Selection creates a tournament with a random sub-set of organisms,
  /// finds the one with the highest fitness, and moves it to the next generation.
  /// User provides the fitness function, the tournament size, and (optionally) the
  /// number of tournaments to run.
  template<typename ORG>
  void TournamentSelect(World<ORG> & world, size_t t_size, size_t tourny_count) {
    emp_assert(t_size > 0 && t_size <= world.GetNumOrgs(), t_size, world.GetNumOrgs());
    emp_assert(tourny_count > 0);

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i < t_size; i++) entries.push_back( world.GetRandomOrgID() );

      double best_fit = world.CalcFitnessID(entries[0]);
      size_t best_id = entries[0];

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < t_size; i++) {
        const double cur_fit = world.CalcFitnessID(entries[i]);
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = entries[i];
        }
      }

      // Place the highest fitness into the next generation!
      world.DoBirth( world[best_id], best_id, 1 );
    }
  }

  /// ==ROULETTE== Selection (aka Fitness-Proportional Selection) chooses organisms to
  /// reproduce based on their current fitness.
  // @CAO: Make sure generations are synchnous, OR update the index map each time through.
  template<typename ORG>
  void RouletteSelect(World<ORG> & world, size_t count=1) {
    emp_assert(count > 0);

    Random & random = world.GetRandom();

    // Load fitnesses from current population.
    IndexMap fitness_index(world.GetSize());
    for (size_t id = 0; id < world.GetSize(); id++) {
      fitness_index.Adjust(id, world.CalcFitnessID(id));
    }

    for (size_t n = 0; n < count; n++) {
      const double fit_pos = random.GetDouble(fitness_index.GetWeight());
      const size_t parent_id = fitness_index.Index(fit_pos);
      const size_t offspring_id = world.DoBirth( world[parent_id], parent_id );
      if (world.IsSynchronous() == false) {
        fitness_index.Adjust(offspring_id, world.CalcFitnessID(offspring_id));
      }
    }
  }

}

#endif
