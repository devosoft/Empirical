/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *
 *  @file  World_select.h
 *  @brief Functions for popular selection methods applied to worlds.
 */

#ifndef EMP_EVO_WORLD_SELECT_H
#define EMP_EVO_WORLD_SELECT_H

#include <map>

#include "../base/array.h"
#include "../base/assert.h"
#include "../base/vector.h"
#include "../tools/IndexMap.h"
#include "../tools/Random.h"

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
    emp_assert(t_size > 0, "Cannot have a tournament with zero organisms.");
    emp_assert(t_size <= world.GetNumOrgs(), "Tournament too big for world.", t_size, world.GetNumOrgs());
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
      world.DoBirth( world.GetGenomeAt(best_id), best_id, 1 );
    }
  }



  /// Function to kill off an organism in DiverseElites to help maintain population size.
  /// Generate a tournament.  Find the two closest together.  Kill the one with the lower fitness.
  template <typename ORG>
  auto DiverseElites_Kill(World<ORG> & world, TraitSet<ORG> traits) {
    emp::vector<Ptr<ORG>> orgs = world.GetFullPop();
    constexpr size_t num_groups = 4;
    emp::array<size_t, num_groups> group_sizes;
    size_t trait_id = 0;

    // Loop until we've found the most dense portion of the population.
    while (orgs.size() > num_groups) {
      // Determine the range on the current trait.
      Trait<ORG, double> & trait = traits[trait_id];
      double min_val = trait.Eval(*orgs[0]);
      double max_val = min_val;
      for (size_t i = 1; i < orgs.size(); i++) {
        double val = trait.Eval(*orgs[i]);
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
      }
      
      // Determine how population divides into groups.
      double trait_width = (max_val - min_val) * 1.0000001;
      double group_width = trait_width / (double) num_groups;
      group_sizes.fill(0);
      for (size_t i = 0; i < orgs.size(); i++) {
        double val = trait.Eval(*orgs[i]);
        size_t group_id = (val - min_val) / group_width;   // @CAO Watch out for zero group_width!
        group_sizes[group_id]++;
      }

      // @CAO: Keep only those orgs in the biggest bin and repeat!

      // Shift to use the next trait.
      if (++trait_id == traits.GetSize()) trait_id = 0;
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
      emp::vector<size_t> order;

      if (max_funs == fit_funs.size()) {
        order = GetPermutation(world.GetRandom(), fit_funs.size());
      } else {
        order.resize(max_funs); // We want to limit the total numebr of tests done.
        for (auto & x : order) x = world.GetRandom().GetUInt(fit_funs.size());
      }
      // @CAO: We could have selected the order more efficiently!

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
      world.DoBirth( world.GetGenomeAt(repro_id), repro_id );
    }
  }

  template<typename ORG>
  struct MapElitesPhenotype {
    using pheno_fun_t = std::function<size_t(const ORG &)>;

    pheno_fun_t pheno_fun;   ///< Function to categorize org into phenotype id.
    size_t id_count;         ///< Numbe of phenotype categories.

    MapElitesPhenotype() : pheno_fun_t(), id_count(0) { ; }
    MapElitesPhenotype(const pheno_fun_t & _f, size_t _ids) : pheno_fun_t(_f), id_count(_ids) { ; }

    bool OK() const { return pheno_fun && id_count; }

    size_t GetID(const ORG & org) const {
      size_t id = pheno_fun(org);
      emp_assert(id < id_count);
      return id;
    }
  };

  template<typename ORG>
  struct MapElitesConfig {
    emp::vector< MapElitesPhenotype<ORG> > phenotypes; ///< Funs to categorizes orgs into phenotypes.

    bool OK() const { for (auto & p : phenotypes) if (!p.OK()) return false; return true; }

    size_t GetID(const ORG & org) const {
      size_t id = 0, scale = 1;
      for (const auto & p : phenotypes) {
        const size_t pid = p.GetID(org);
        id += pid * scale;
        scale *= p.id_count;
      }
      return id;
    }

    size_t GetIDCount() const {
      size_t id_count = 1;
      for (const auto & p : phenotypes) id_count *= p.id_count;
      return id_count;
    }
  };

  /// ==MAP-ELITES== Add a new organism to MapElites.  Selection looks at multiple phenotypic
  /// traits and keeps only the highest fitness with each combination of traits.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param config Information about the pheonotypes that Map Elites needs to use.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  template<typename ORG>
  void MapElitesSeed(World<ORG> & world,
                     const MapElitesConfig<ORG> & config,
                     const ORG & org)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(config.OK());
  }

  /// ==MAP-ELITES== Replicate a random organism in MapElites.  Selection looks at multiple
  /// phenotypic traits and keeps only the highest fitness with each combination of traits.
  /// @param world The emp::World object with the organisms to be selected.
  /// @param config Information about the pheonotypes that Map Elites needs to use.
  /// @param repro_count How many rounds of repliction should we do. (default 1)
  template<typename ORG>
  void MapElitesGrow(World<ORG> & world,
                     const MapElitesConfig<ORG> & config,
                     size_t repro_count=1)
  {
    emp_assert(world.GetSize() > 0);
    emp_assert(config.OK());
  }

}

#endif
