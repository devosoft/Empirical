/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  World_select.hpp
 *  @brief Functions for popular selection methods applied to worlds.
 */

#ifndef EMP_EVO_WORLD_SELECT_H
#define EMP_EVO_WORLD_SELECT_H

#include <map>
#include <functional>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/macros.hpp"
#include "../base/vector.hpp"
#include "../datastructs/IndexMap.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../math/Random.hpp"
#include "../meta/reflection.hpp"

namespace emp {

  template<typename ORG> class World;

  /// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
  /// the next generation.  Find top e_count individuals and make copy_count copies of each.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param e_count How many distinct organisms should be chosen, starting from the most fit.
  /// @param copy_count How many copies should be made of each elite organism?
  template<typename ORG>
  void EliteSelect(World<ORG> & world, size_t e_count=1, size_t copy_count=1) {
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
      world.DoBirth( world.GetGenomeAt(repro_id), repro_id, copy_count);
      ++m;
    }
  }

  /// ==RANDOM== Selection picks an organism with uniform-random probability form the populaiton.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param r_count How many distinct organisms should be chosen?
  /// @param copy_count How many copies should be made of each chosen organism?
  template<typename ORG>
  void RandomSelect(World<ORG> & world, size_t r_count=1, size_t copy_count=1) {
    emp_assert(r_count > 0, r_count);
    emp_assert(copy_count > 0);

    Random & random = world.GetRandom();

    // Pick r_count organisms;
    for (size_t i = 0; i < r_count; i++) {
      // Choose an organism at random
      size_t id = random.GetUInt(world.GetSize());
      while (world.IsOccupied(id) == false) id = random.GetUInt(world.GetSize());

      // Make copy_count copies.
      world.DoBirth( world.GetGenomeAt(id), id, copy_count);
    }
  }

  /// ==TOURNAMENT== Selection creates a tournament with a random sub-set of organisms,
  /// finds the one with the highest fitness, and moves it to the next generation.
  /// User provides the world (with a fitness function), the tournament size, and
  /// (optionally) the number of tournaments to run.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param t_size How many organisms should be placed in each tournament?
  /// @param tourny_count How many tournaments should be run? (with replacement of organisms)
  template<typename ORG>
  void TournamentSelect(World<ORG> & world, size_t t_size, size_t tourny_count=1) {
    emp_assert(t_size > 0, "Cannot have a tournament with zero organisms.", t_size, world.GetNumOrgs());
    emp_assert(t_size <= world.GetNumOrgs(), "Tournament too big for world.", t_size, world.GetNumOrgs());
    emp_assert(tourny_count > 0);

    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      // Choose organisms for this tournament (with replacement!)
      for (size_t i=0; i < t_size; i++) entries.push_back(
        world.GetRandomOrgID()
      );

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
      world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
    }
  }

  /// ==LOCAL TOURNAMENT== Selection creates a tournament with a random sub-set of organisms that are neighbor to a random organism,
  /// finds the one with the highest fitness, and moves it to the next generation.
  /// User provides the world (with a fitness function), the tournament size, and
  /// (optionally) the number of tournaments to run.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param t_size How many organisms should be placed in each tournament?
  /// @param tourny_count How many tournaments should be run? (with replacement of organisms)
  template<typename ORG>
  void LocalTournamentSelect(World<ORG> & world, size_t t_size, size_t tourny_count=1) {
    emp_assert(t_size > 0, "Cannot have a tournament with zero organisms.", t_size, world.GetNumOrgs());
    emp_assert(t_size <= world.GetNumOrgs(), "Tournament too big for world.", t_size, world.GetNumOrgs());
    emp_assert(tourny_count > 0);

    for (size_t T = 0; T < tourny_count; T++) {
      emp::vector<size_t> entries{world.GetRandomOrgID()};

      // Choose organisms for this tournament (with replacement!)
      for (size_t i = 1; i < t_size; i++) entries.push_back(
        world.GetRandomNeighborPos(entries[0]).GetPopID()
      );

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
      world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
    }
  }


  /// ==ROULETTE== Selection (aka Fitness-Proportional Selection) chooses organisms to
  /// reproduce based on their current fitness.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param count How many organims should be selected for replication? (with replacement)
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
      const size_t offspring_id = world.DoBirth( world.GetGenomeAt(parent_id), parent_id ).GetIndex();
      if (world.IsSynchronous() == false) {
        fitness_index.Adjust(offspring_id, world.CalcFitnessID(offspring_id));
      }
    }
  }


  EMP_CREATE_OPTIONAL_METHOD(TriggerOnLexicaseSelect, TriggerOnLexicaseSelect);

  /// ==LEXICASE== Selection runs through multiple fitness functions in a random order for
  /// EACH offspring produced.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param fit_funs The set of fitness functions to shuffle for each organism reproduced.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  /// @param max_funs The maximum number of fitness functions to use. (use 0 for all; default)
  template<typename ORG>
  void LexicaseSelect(World<ORG> & world,
                      const emp::vector< std::function<double(const ORG &)> > & fit_funs,
                      size_t repro_count=1,
                      size_t max_funs=0)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(fit_funs.size() > 0);

    if (!max_funs) max_funs = fit_funs.size();

    // Determine which positions are occupied.
    emp::vector<size_t> all_orgs(world.GetSize()), cur_orgs, next_orgs;
    for (size_t org_id = 0; org_id < world.GetSize(); org_id++) all_orgs[org_id] = org_id;

    // Pre-calculate all fitness info.
    // (@CAO: Technically should only do this if caching is turned on?)
    // (@CAO: Also, we should only pre-calculate if we think we need it all.  Lazy eval better?)
    emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
    for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
      fitnesses[fit_id].resize(world.GetSize());
      for (size_t org_id : all_orgs) {
        fitnesses[fit_id][org_id] = fit_funs[fit_id](world.GetOrg(all_orgs[org_id]));
      }
    }

    for (size_t repro = 0; repro < repro_count; ++repro) {
      // Determine the current ordering of the functions.
      // (@CAO: This can be more efficient)
      emp::vector<size_t> order(max_funs);
      if (max_funs == fit_funs.size()) {
        order = GetPermutation(world.GetRandom(), fit_funs.size());
      } else {
        for (auto & x : order) x = world.GetRandom().GetUInt(fit_funs.size());
      }

      // Step through the functions in the proper order.
      cur_orgs = all_orgs;  // Start with all of the organisms.
      int depth = -1;
      for (size_t fit_id : order) {
        depth++;

        double max_fit = fitnesses[fit_id][cur_orgs[0]];
        next_orgs.push_back(cur_orgs[0]);

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
      size_t win_id = cur_orgs[ world.GetRandom().GetUInt(cur_orgs.size()) ];

      //TriggerOnLexicaseSelect(world, used, win_id);
      world.DoBirth( world.GetGenomeAt(win_id), win_id );
    }
    // std::cout << "Done with lex" << std::endl;
  }

  /// ==OPTIMIZED LEXICASE== Is the same as regular lexicase, but determines how many unique genotypes there are
  //  ahead of time and performs each analysis only once.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param fit_funs The set of fitness functions to shuffle for each organism reproduced.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  /// @param max_funs The maximum number of fitness functions to use. (use 0 for all; default)
  template<typename ORG>
  void OptimizedLexicaseSelect(World<ORG> & world,
                      const emp::vector< std::function<double(const ORG &)> > & fit_funs,
                      size_t repro_count=1,
                      size_t max_funs=0)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(fit_funs.size() > 0);

    // @CAO: Can probably optimize a bit!

    std::map<typename decltype(World<ORG>())::genome_t, int> genotype_counts;
    emp::vector<emp::vector<size_t>> genotype_lists;

    // Find all orgs with same genotype - we can dramatically reduce
    // fitness evaluations this way.
    for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
      if (world.IsOccupied(org_id)) {
        const typename decltype(World<ORG>())::genome_t gen = world.GetGenomeAt(org_id);
        if (emp::Has(genotype_counts, gen)) {
          genotype_lists[genotype_counts[gen]].push_back(org_id);
        } else {
          genotype_counts[gen] = genotype_lists.size();
          genotype_lists.emplace_back(emp::vector<size_t>{org_id});
        }
      }
    }

    emp::vector<size_t> all_gens(genotype_lists.size()), cur_gens, next_gens;

    for (size_t i = 0; i < genotype_lists.size(); i++) {
      all_gens[i] = i;
    }

    if (!max_funs) max_funs = fit_funs.size();
    // std::cout << "in lexicase" << std::endl;
    // Collect all fitness info. (@CAO: Technically only do this is cache is turned on?)
    emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
    for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
      fitnesses[fit_id].resize(genotype_counts.size());
      // std::cout << fit_id << std::endl;
      int id = 0;
      for (auto & gen : genotype_lists) {
        fitnesses[fit_id][id] = fit_funs[fit_id](world.GetOrg(gen[0]));
        id++;
      }
    }

    // std::cout << to_string(fitnesses) << std::endl;
  // std::cout << "fitness calculated" << std::endl;
    // Go through a new ordering of fitness functions for each selections.
  // std::cout << "randdomized" << std::endl;
    for (size_t repro = 0; repro < repro_count; ++repro) {
  // std::cout << "repro: " << repro << std::endl;
      // Determine the current ordering of the functions.
      emp::vector<size_t> order;

      if (max_funs == fit_funs.size()) {
        order = GetPermutation(world.GetRandom(), fit_funs.size());
      } else {
        order.resize(max_funs); // We want to limit the total numebr of tests done.
        for (auto & x : order) x = world.GetRandom().GetUInt(fit_funs.size());
      }
      // @CAO: We could have selected the order more efficiently!
  // std::cout << "reoreder" << std::endl;
      // Step through the functions in the proper order.
      cur_gens = all_gens;  // Start with all of the organisms.
      int depth = -1;
      for (size_t fit_id : order) {
        // std::cout << "fit_id: " << fit_id << std::endl;
        depth++;

        // std::cout << "about to index" << std::endl;
        // std::cout << to_string(fitnesses[fit_id]) << std::endl;
        // std::cout << cur_orgs[0] << std::endl;
        double max_fit = fitnesses[fit_id][cur_gens[0]];
        next_gens.push_back(cur_gens[0]);
        // std::cout << "Starting max: " << max_fit << to_string(cur_gens) << std::endl;
        for (size_t gen_id : cur_gens) {

          const double cur_fit = fitnesses[fit_id][gen_id];
          // std::cout << "gen_id: " << gen_id << "Fit: " << cur_fit << std::endl;
          if (cur_fit > max_fit) {
            max_fit = cur_fit;             // This is a the NEW maximum fitness for this function
            next_gens.resize(0);           // Clear out orgs with former maximum fitness
            next_gens.push_back(gen_id);   // Add this org as only one with new max fitness
            // std::cout << "New max: " << max_fit << " " << cur_gens.size() << std::endl;
          }
          else if (cur_fit == max_fit) {
            next_gens.push_back(gen_id);   // Same as cur max fitness -- save this org too.
            // std::cout << "Adding: " << gen_id << std::endl;
          }
        }
        // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
        std::swap(cur_gens, next_gens);
        next_gens.resize(0);

        if (cur_gens.size() == 1) break;  // Stop if we're down to just one organism.
      }

      // Place a random survivor (all equal) into the next generation!
      emp_assert(cur_gens.size() > 0, cur_gens.size(), fit_funs.size(), all_gens.size());
      size_t options = 0;
      for (size_t gen : cur_gens) {
        options += genotype_lists[gen].size();
      }
      size_t winner = world.GetRandom().GetUInt(options);
      int repro_id = -1;

      for (size_t gen : cur_gens) {
        if (winner < genotype_lists[gen].size()) {
          repro_id = (int) genotype_lists[gen][winner];
          break;
        }
        winner -= genotype_lists[gen].size();
      }
      emp_assert(repro_id != -1, repro_id, winner, options);

      // std::cout << depth << "about to calc used" <<std::endl;
      emp::vector<size_t> used = Slice(order, 0, depth+1);
      // If the world has a OnLexicaseSelect method, call it
      // std::cout << depth << " " << to_string(used) << std::endl;
      TriggerOnLexicaseSelect(world, used, repro_id);
      world.DoBirth( world.GetGenomeAt(repro_id), repro_id );
    }
    // std::cout << "Done with lex" << std::endl;
  }




  // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
  // functions.  The best individuals on each supplemental function divide up a resource pool.
  // NOTE: You must turn off the FitnessCache for this function to work properly.
  template<typename ORG>
  void EcoSelect(World<ORG> & world, const emp::vector<std::function<double(ORG &)> > & extra_funs,
                  const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
  {
    emp_assert(world.GetFitFun(), "Must define a base fitness function");
    emp_assert(world.GetSize() > 0);
    emp_assert(t_size > 0 && t_size <= world.GetSize(), t_size, world.GetSize());
    // emp_assert(world.IsCacheOn() == false, "Ecologies mean constantly changing fitness!");

    if (world.IsCacheOn()) {
        world.ClearCache();
    }

    // Setup info to track fitnesses.
    emp::vector<double> base_fitness(world.GetSize());
    emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
    emp::vector<double> max_extra_fit(extra_funs.size(), 0.0);
    emp::vector<size_t> max_count(extra_funs.size(), 0);
    for (size_t i=0; i < extra_funs.size(); i++) {
      extra_fitnesses[i].resize(world.GetSize());
    }

    // Collect all fitness info.
    for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
      base_fitness[org_id] = world.CalcFitnessID(org_id);
      for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
        double cur_fit = extra_funs[ex_id](world.GetOrg(org_id));
        extra_fitnesses[ex_id][org_id] = cur_fit;
        if (cur_fit > max_extra_fit[ex_id]) {
          max_extra_fit[ex_id] = cur_fit;
          max_count[ex_id] = 1;
        }
        else if (cur_fit == max_extra_fit[ex_id]) {
          max_count[ex_id]++;
        }
      }
    }

    // Readjust base fitness to reflect extra resources.
    for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
      if (max_count[ex_id] == 0) continue;  // No one gets this reward...

      // The current bonus is divided up among the organisms that earned it...
      const double cur_bonus = pool_sizes[ex_id] / max_count[ex_id];
      // std::cout << "Bonus " << ex_id << " = " << cur_bonus
      //           << "   max_extra_fit = " << max_extra_fit[ex_id]
      //           << "   max_count = " << max_count[ex_id]
      //           << std::endl;

      for (size_t org_id = 0; org_id < world.GetSize(); org_id++) {
        // If this organism is the best at the current resource, git it the bonus!
        if (extra_fitnesses[ex_id][org_id] == max_extra_fit[ex_id]) {
          base_fitness[org_id] += cur_bonus;
        }
      }
    }


    emp::vector<size_t> entries;
    for (size_t T = 0; T < tourny_count; T++) {
      entries.resize(0);
      for (size_t i=0; i<t_size; i++) entries.push_back( world.GetRandomOrgID() ); // Allows replacement!

      double best_fit = base_fitness[entries[0]];
      size_t best_id = entries[0];

      // Search for a higher fit org in the tournament.
      for (size_t i = 1; i < t_size; i++) {
        const double cur_fit = base_fitness[entries[i]];
        if (cur_fit > best_fit) {
          best_fit = cur_fit;
          best_id = entries[i];
        }
      }

      // Place the highest fitness into the next generation!
      world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
    }
  }

  /// EcoSelect can be provided a single value if all pool sizes are identical.
  template<typename ORG>
  void EcoSelect(World<ORG> & world, const emp::vector<typename World<ORG>::fun_calc_fitness_t > & extra_funs,
                  double pool_sizes, size_t t_size, size_t tourny_count=1)
  {
    emp::vector<double> pools(extra_funs.size(), pool_sizes);
    EcoSelect(world, extra_funs, pools, t_size, tourny_count);
  }

}

#endif
