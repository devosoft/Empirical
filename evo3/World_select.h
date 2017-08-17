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
#include "../base/vector.h"
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


  /// ==LEXICASE== Selection runs through multiple fitness functions in a random order for
  /// EACH offspring produced.
  template<typename ORG>
  void LexicaseSelect(World<ORG> & world,
                      const emp::vector< std::function<double(ORG&)> > & fit_funs,
                      size_t repro_count=1)
  {
    emp_assert(pop.size() > 0);
    emp_assert(fit_funs.size() > 0);

    // Collect all fitness info. (@CAO: Technically only do this is cache is turned on?)
    emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
    for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
      fitnesses[fit_id].resize(world.GetSize());
      for (size_t org_id = 0; org_id < world.GetSize(); ++org_id) {
        fitnesses[fit_id][org_id] = fit_funs[fit_id](world[org_id]);
      }
    }

    // Go through a new ordering of fitness functions for each selections.
    // @CAO: Can probably optimize a bit!
    emp::vector<size_t> all_orgs(world.GetSize()), cur_orgs, next_orgs;
    for (size_t org_id = 0; org_id < world.GetSize(); org_id++) all_orgs[org_id] = org_id;

    for (size_t repro = 0; repro < repro_count; ++repro) {
      // Determine the current ordering of the functions.
      emp::vector<size_t> order = GetPermutation(world.GetRandom(), fit_funs.size());

      // Step through the functions in the proper order.
      cur_orgs = all_orgs;  // Start with all of the organisms.
      for (size_t fit_id : order) {
        double max_fit = fitnesses[fit_id][cur_orgs[0]];
        for (size_t org_id : cur_orgs) {
          const double cur_fit = fitnesses[fit_id][org_id];
          if (cur_fit > max_fit) {
            max_fit = cur_fit;             // This is a the NEW maximum fitness for this function
            next_orgs.resize(0);           // Clear out orgs with former maximum fitness
            next_orgs.push_back(org_id);   // Add this org as only one with new max fitness
          }
          else if (cur_fit == max_fit) {
            next_orgs.push_back(org_id);   // Same as cur max fitness -- save this org too.
          }
        }
        // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
        std::swap(cur_orgs, next_orgs);
        next_orgs.resize(0);

        if (cur_orgs.size() == 1) break;  // Stop if we're down to just one organism.
      }

      // Place a random survivor (all equal) into the next generation!
      emp_assert(cur_orgs.size() > 0, cur_orgs.size(), fit_funs.size(), all_orgs.size());
      size_t repro_id = cur_orgs[ world.GetRandom().GetUInt(cur_orgs.size()) ];
      InsertBirth( world[repro_id], repro_id, 1 );
    }
  }

}

#endif
