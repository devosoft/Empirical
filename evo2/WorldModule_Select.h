//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Selection mechanisms choose organisms for the next generation.

#ifndef EMP_EVO_WORLD_MODULE_SELECT_H
#define EMP_EVO_WORLD_MODULE_SELECT_H

namespace emp {

  template <typename ORG, emp::evo... MODS>
  class WorldModule<ORG, evo::Select, MODS...> : public WorldModule<ORG, MODS...> {
  protected:
    // Parent-related types.
    using parent_t = WorldModule<ORG, MODS...>;
    using fit_fun_t = typename parent_t::fit_fun_t;

    // Parent member vars.
    using parent_t::default_fit_fun;
    using parent_t::pop;

    // Parent member functions
    using parent_t::CalcFitness;
    using parent_t::IsOccupied;

  public:

    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(const fit_fun_t & fit_fun, size_t e_count=1, size_t copy_count=1) {
      emp_assert(fit_fun);
      emp_assert(e_count > 0 && e_count <= pop.size());
      emp_assert(copy_count > 0);

      // Load the population into a multimap, sorted by fitness.
      std::multimap<double, size_t> fit_map;
      for (size_t i = 0; i < pop.size(); i++) {
        if (IsOccupied(i)){
          const double cur_fit = CalcFitness(*pop[i], fit_fun);
          fit_map.insert( std::make_pair(cur_fit, i) );
        }
      }

      // Grab the top fitnesses and move them into the next generation.
      auto m = fit_map.rbegin();
      for (size_t i = 0; i < e_count; i++) {
        this->InsertBirth( *(pop[m->second]), m->second, copy_count);
        ++m;
      }
    }

    // Elite Selection can use the default fitness function.
    void EliteSelect(size_t e_count=1, size_t copy_count=1) {
      EliteSelect(default_fit_fun, e_count, copy_count);
    }

  //   // Roulette Selection (aka Fitness-Proportional Selection) chooses organisms to
  //   // reproduce based on their current fitness.
  //   void RouletteSelect(const fit_fun_t & fit_fun, size_t count=1) {
  //     emp_assert(fit_fun);
  //     emp_assert(count > 0);
  //     emp_assert(random_ptr != nullptr && "RouletteSelect() requires active random_ptr");
  //
  //     pop.CalcFitnessAll(fit_fun);
  //
  //     for (size_t n = 0; n < count; n++) {
  //       const double fit_pos = random_ptr->GetDouble(fitM.GetTotalFitness());
  //       size_t id = fitM.At(fit_pos);
  //       InsertBirth( *(pop[id]), id, 1, fit_fun );
  //     }
  //   }
  //
  //   // Tournament Selection creates a tournament with a random sub-set of organisms,
  //   // finds the one with the highest fitness, and moves it to the next generation.
  //   // User provides the fitness function, the tournament size, and (optionally) the
  //   // number of tournaments to run.
  //   void TournamentSelect(const fit_fun_t & fit_fun, size_t t_size, size_t tourny_count=1) {
  //     emp_assert(fit_fun);
  //     emp_assert(t_size > 0 && t_size <= pop.size(), t_size, pop.size());
  //     emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");
  //
  //     emp::vector<size_t> entries;
  //     for (size_t T = 0; T < tourny_count; T++) {
  //       entries.resize(0);
  //       for (size_t i=0; i<t_size; i++) entries.push_back( GetRandomOrg() ); // Allows replacement!
  //
  //       double best_fit = popM.CalcFitness(entries[0], fit_fun);
  //       size_t best_id = entries[0];
  //
  //       // Search for a higher fit org in the tournament.
  //       for (size_t i = 1; i < t_size; i++) {
  //         const double cur_fit = popM.CalcFitness(entries[i], fit_fun);
  //         if (cur_fit > best_fit) {
  //           best_fit = cur_fit;
  //           best_id = entries[i];
  //         }
  //       }
  //
  //       // Place the highest fitness into the next generation!
  //       InsertBirth( *(popM[best_id]), best_id, 1 );
  //     }
  //   }
  //
  //   // Tournament Selection can use the default fitness function.
  //   void TournamentSelect(size_t t_size, size_t tourny_count=1) {
  //     TournamentSelect(orgM.GetFitFun(), t_size, tourny_count);
  //   }
  //
  //
  //   // Run tournament selection with fitnesses adjusted by Goldberg and
  //   // Richardson's fitness sharing function (1987)
  //   // Requires a distance function that is valid for members of the population,
  //   // a sharing threshold (sigma share) that defines which members are
  //   // in the same niche, and a value of alpha (which controls the shape of
  //   // the fitness sharing curve
  //   void FitnessSharingTournamentSelect(const fit_fun_t & fit_fun, const dist_fun_t & dist_fun,
	// 				double sharing_threshhold, double alpha, size_t t_size, size_t tourny_count=1)
  //   {
  //     emp_assert(t_size > 0 && t_size <= popM.size());
  //
  //     // Pre-calculate fitnesses.
  //     emp::vector<double> fitness(popM.size());
  //     for (size_t i = 0; i < popM.size(); ++i) {
  //       double niche_count = 0;
  //       for (size_t j = 0; j < popM.size(); ++j) {
  //         double dij = dist_fun(popM[i], popM[j]);
  //         niche_count += std::max(1 - std::pow(dij/sharing_threshhold, alpha), 0.0);
  //       }
  //       fitness[i] = fit_fun(popM[i])/niche_count;
  //     }
  //
  //     fitM.Set(fitness);                                // Cache all calculated fitnesses.
  //     TournamentSelect(fit_fun, t_size, tourny_count);
  //   }
  //
  //   // Fitness sharing Tournament Selection can use the default fitness function
  //   void FitnessSharingTournamentSelect(const dist_fun_t & dist_fun, double sharing_threshold,
	// 				double alpha, size_t t_size, size_t tourny_count=1) {
  //     FitnessSharingTournamentSelect(orgM.GetFitFun(), dist_fun, sharing_threshold, alpha, t_size, tourny_count);
  //   }
  //
  //
  //   // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
  //   // functions.  The best individuals on each supplemental function divide up a resource pool.
  //   // NOTE: You must turn off the FitnessCache for this function to work properly.
  //   void EcoSelect(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
  //                  const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
  //   {
  //     emp_assert(fit_fun);
  //     emp_assert(t_size > 0 && t_size <= popM.size(), t_size, popM.size());
  //     emp_assert(random_ptr != nullptr && "EcoSelect() requires active random_ptr");
  //     emp_assert(fitM.IsCached() == false, "Ecologies mean constantly changing fitness!");
  //
  //     // Setup info to track fitnesses.
  //     emp::vector<double> base_fitness(popM.size());
  //     emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
  //     emp::vector<double> max_extra_fit(extra_funs.size(), 0.0);
  //     emp::vector<size_t> max_count(extra_funs.size(), 0);
  //     for (size_t i=0; i < extra_funs.size(); i++) {
  //       extra_fitnesses[i].resize(popM.size());
  //     }
  //
  //     // Collect all fitness info.
  //     for (size_t org_id = 0; org_id < popM.size(); org_id++) {
  //       base_fitness[org_id] = popM.CalcFitness(org_id, fit_fun);
  //       for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
  //         double cur_fit = popM.CalcFitness(org_id, extra_funs[ex_id]);
  //         extra_fitnesses[ex_id][org_id] = cur_fit;
  //         if (cur_fit > max_extra_fit[ex_id]) {
  //           max_extra_fit[ex_id] = cur_fit;
  //           max_count[ex_id] = 1;
  //         }
  //         else if (cur_fit == max_extra_fit[ex_id]) {
  //           max_count[ex_id]++;
  //         }
  //       }
  //     }
  //
  //     // Readjust base fitness to reflect extra resources.
  //     for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
  //       if (max_count[ex_id] == 0) continue;  // No one gets this reward...
  //
  //       // The current bonus is divided up among the organisms that earned it...
  //       const double cur_bonus = pool_sizes[ex_id] / (double) max_count[ex_id];
  //       // std::cout << "Bonus " << ex_id << " = " << cur_bonus
  //       //           << "   max_extra_fit = " << max_extra_fit[ex_id]
  //       //           << std::endl;
  //
  //       for (size_t org_id = 0; org_id < popM.size(); org_id++) {
  //         // If this organism is the best at the current resource, git it the bonus!
  //         if (extra_fitnesses[ex_id][org_id] == max_extra_fit[ex_id]) {
  //           base_fitness[org_id] += cur_bonus;
  //         }
  //       }
  //     }
  //
  //
  //     emp::vector<size_t> entries;
  //     for (size_t T = 0; T < tourny_count; T++) {
  //       entries.resize(0);
  //       for (size_t i=0; i<t_size; i++) entries.push_back( popM.GetRandomOrg() ); // Allows replacement!
  //
  //       double best_fit = base_fitness[entries[0]];
  //       size_t best_id = entries[0];
  //
  //       // Search for a higher fit org in the tournament.
  //       for (size_t i = 1; i < t_size; i++) {
  //         const double cur_fit = base_fitness[entries[i]];
  //         if (cur_fit > best_fit) {
  //           best_fit = cur_fit;
  //           best_id = entries[i];
  //         }
  //       }
  //
  //       // Place the highest fitness into the next generation!
  //       InsertBirth( *(popM[best_id]), best_id, 1 );
  //     }
  //   }
  //
  //   /// EcoSelect can be provided a single value if all pool sizes are identical.
  //   void EcoSelect(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
  //                  double pool_sizes, size_t t_size, size_t tourny_count=1)
  //   {
  //     emp::vector<double> pools(extra_funs.size(), pool_sizes);
  //     EcoSelect(fit_fun, extra_funs, pools, t_size, tourny_count);
  //   }
  //
  //   // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
  //   // functions.  The best individuals on each supplemental function divide up a resource pool.
  //   // NOTE: You must turn off the FitnessCache for this function to work properly.
  //   void EcoSelectGradation(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
	// 		    const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
  //   {
  //     emp_assert(fit_fun);
  //     emp_assert(t_size > 0 && t_size <= popM.size(), t_size, popM.size());
  //     emp_assert(random_ptr != nullptr && "EcoSelect() requires active random_ptr");
  //     emp_assert(fitM.IsCached() == false, "Ecologies mean constantly changing fitness!");
  //
  //     // Setup info to track fitnesses.
  //     emp::vector<double> base_fitness(popM.size());
  //     emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
  //     emp::vector<double> max_extra_fit(extra_funs.size(), 0.0);
  //     emp::vector<double> resource_left = pool_sizes;
  //     emp::vector<size_t> max_count(extra_funs.size(), 0);
  //     for (size_t i=0; i < extra_funs.size(); i++) {
  //       extra_fitnesses[i].resize(popM.size());
  //     }
  //     emp::vector<size_t> ordering(popM.size());
  //
  //
  //     // Collect all fitness info.
  //     for (size_t org_id = 0; org_id < popM.size(); org_id++) {
  //       base_fitness[org_id] = popM.CalcFitness(org_id, fit_fun);
  //       ordering[org_id] = org_id;
  //       for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
  //         double cur_fit = popM.CalcFitness(org_id, extra_funs[ex_id]);
  //         extra_fitnesses[ex_id][org_id] = Pow2(cur_fit/32);
  //       }
  //     }
  //
  //     for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
	// std::sort(ordering.begin(), ordering.end(),
	// 	  [&extra_fitnesses, &ex_id](int x, int y){
	// 	    return extra_fitnesses[ex_id][(size_t)x] > extra_fitnesses[ex_id][(size_t)y];
	// 	  });
	// for (size_t org_id : ordering) {
	//   double bonus = .05 * extra_fitnesses[ex_id][org_id] * resource_left[ex_id];
	//   extra_fitnesses[ex_id][org_id] = bonus;
	//   resource_left[ex_id] -= bonus;
	//   base_fitness[org_id] += bonus;
	// }
  //
  //     }
  //
  //     //   for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
  //     //     std::cout << "Bonus " << ex_id << " = " << extra_fitnesses[ex_id]
  //     //               << std::endl;
  //     //   }
  //
  //
  //     emp::vector<size_t> entries;
  //     for (size_t T = 0; T < tourny_count; T++) {
  //       entries.resize(0);
  //       for (size_t i=0; i<t_size; i++) entries.push_back( popM.GetRandomOrg() ); // Allows replacement!
  //
  //       double best_fit = base_fitness[entries[0]];
  //       size_t best_id = entries[0];
  //
  //       // Search for a higher fit org in the tournament.
  //       for (size_t i = 1; i < t_size; i++) {
  //         const double cur_fit = base_fitness[entries[i]];
  //         if (cur_fit > best_fit) {
  //           best_fit = cur_fit;
  //           best_id = entries[i];
  //         }
  //       }
  //
  //       // Place the highest fitness into the next generation!
  //       InsertBirth( *(popM[best_id]), best_id, 1 );
  //     }
  //   }
  //
  //   /// EcoSelect can be provided a single value if all pool sizes are identical.
  //   void EcoSelectGradation(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
	// 		    double pool_sizes, size_t t_size, size_t tourny_count=1)
  //   {
  //     emp::vector<double> pools(extra_funs.size(), pool_sizes);
  //     EcoSelectGradation(fit_fun, extra_funs, pools, t_size, tourny_count);
  //   }
  //
  //   /// LexicaseSelect runs through multiple fitness functions in a random order for
  //   /// EACH offspring produced.
  //   // NOTE: You must turn off the FitnessCache for this function to work properly.
  //   void LexicaseSelect(const emp::vector<fit_fun_t> & fit_funs, size_t repro_count=1)
  //   {
  //     emp_assert(popM.size() > 0);
  //     emp_assert(fit_funs.size() > 0);
  //     emp_assert(random_ptr != nullptr && "LexicaseSelect() requires active random_ptr");
  //     emp_assert(fitM.IsCached() == false, "Lexicase constantly changes fitness functions!");
  //
  //     // Collect all fitness info.
  //     emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
  //     for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
  //       fitnesses[fit_id].resize(popM.size());
  //       for (size_t org_id = 0; org_id < popM.size(); ++org_id) {
  //         fitnesses[fit_id][org_id] = popM.CalcFitness(org_id, fit_funs[fit_id]);
  //       }
  //     }
  //
  //     // Go through a new ordering of fitness functions for each selections.
  //     // @CAO: Can probably optimize a bit!
  //     // std::set<emp::vector<size_t>> permute_set;
  //     emp::vector<size_t> all_orgs(popM.size()), cur_orgs, next_orgs;
  //     for (size_t org_id = 0; org_id < popM.size(); org_id++) all_orgs[org_id] = org_id;
  //
  //     for (size_t repro = 0; repro < repro_count; ++repro) {
  //       // Determine the current ordering of the functions.
	// emp::vector<size_t> order = GetPermutation(*random_ptr, fit_funs.size());
  //
  //       // Step through the functions in the proper order.
  //       cur_orgs = all_orgs;  // Start with all of the organisms.
  //       for (size_t fit_id : order) {
  //         double max_fit = fitnesses[fit_id][cur_orgs[0]];
  //         for (size_t org_id : cur_orgs) {
  //           const double cur_fit = fitnesses[fit_id][org_id];
  //           if (cur_fit > max_fit) {
  //             max_fit = cur_fit;             // This is a the NEW maximum fitness for this function
  //             next_orgs.resize(0);           // Clear out orgs with former maximum fitness
  //             next_orgs.push_back(org_id);   // Add this org as only one with new max fitness
  //           }
  //           else if (cur_fit == max_fit) {
  //             next_orgs.push_back(org_id);   // Same as cur max fitness -- save this org too.
  //           }
  //         }
  //         // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
	//   std::swap(cur_orgs, next_orgs);
  //         next_orgs.resize(0);
  //       }
  //
  //       // Place a random survivor (all equal) into the next generation!
  //       emp_assert(cur_orgs.size() > 0, cur_orgs.size(), fit_funs.size(), all_orgs.size());
  //       size_t repro_id = cur_orgs[ random_ptr->GetUInt(cur_orgs.size()) ];
  //       InsertBirth( *(popM[repro_id]), repro_id, 1 );
  //     }
  //   }
  //
  //   /// An Ecological version of Lexicase selection.
  //   // NOTE: You must turn off the FitnessCache for this function to work properly.
  //   void EcocaseSelect(const emp::vector<fit_fun_t> & fit_funs,
  //                      const emp::vector<double> & probs,   // Probability of using each function.
  //                      double repro_decay,                  // Reduced chance of being used again.
  //                      size_t repro_count)
  //   {
  //     emp_assert(popM.size() > 0);
  //     emp_assert(fit_funs.size() > 0 && fit_funs.size() == probs.size());
  //     emp_assert(repro_decay > 0.0 && repro_decay <= 1.0, repro_decay);
  //     emp_assert(random_ptr != nullptr && "EcocaseSelect() requires active random_ptr");
  //     emp_assert(fitM.IsCached() == false, "Ecocase constantly changes fitness functions!");
  //
  //     // Collect all fitness info.
  //     emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
  //     for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
  //       fitnesses[fit_id].resize(popM.size());
  //       for (size_t org_id = 0; org_id < popM.size(); ++org_id) {
  //         fitnesses[fit_id][org_id] = popM.CalcFitness(org_id, fit_funs[fit_id]);
  //       }
  //     }
  //
  //     // Determine how eligible each org is for being reproduced.
  //     emp::vector<double> repro_prob(popM.size(), 1.0);
  //
  //     // Go through a new ordering of fitness functions for each selection.
  //     emp::vector<size_t> all_orgs(popM.size()), cur_orgs, next_orgs;
  //     for (size_t org_id = 0; org_id < popM.size(); org_id++) all_orgs[org_id] = org_id;
  //
  //     for (size_t repro = 0; repro < repro_count; ++repro) {
  //       // Determine the current ordering of the functions.
	// emp::vector<size_t> order = GetPermutation(*random_ptr, fit_funs.size());
  //
  //       // Determine the starting set of organisms.
  //       cur_orgs.resize(0);
  //       for (size_t org_id = 0; org_id < popM.size(); org_id++) {
  //         if (repro_prob[org_id] == 1.0 || random_ptr->P(repro_prob[org_id])) {
  //           cur_orgs.push_back(org_id);
  //         }
  //       }
  //
  //       // Step through the functions in the proper order.
  //       for (size_t fit_id : order) {
  //         // Determine if we should skip this fitness function.
  //         if ( random_ptr->P(1.0 - probs[fit_id]) ) continue;
  //         double max_fit = fitnesses[fit_id][cur_orgs[0]];
  //         for (size_t org_id : cur_orgs) {
  //           const double cur_fit = fitnesses[fit_id][org_id];
  //           if (cur_fit > max_fit) {
  //             max_fit = cur_fit;             // This is a the NEW maximum fitness for this function
  //             next_orgs.resize(0);           // Clear out orgs with former maximum fitness
  //             next_orgs.push_back(org_id);   // Add this org as only one with new max fitness
  //           }
  //           else if (cur_fit == max_fit) {
  //             next_orgs.push_back(org_id);   // Same as cur max fitness -- save this org too.
  //           }
  //         }
  //         // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
	//   std::swap(cur_orgs, next_orgs);
  //         next_orgs.resize(0);
  //       }
  //
  //       // Place a random survivor (all equal) into the next generation!
  //       emp_assert(cur_orgs.size() > 0, cur_orgs.size(), fit_funs.size(), all_orgs.size());
  //       size_t repro_id = cur_orgs[ random_ptr->GetUInt(cur_orgs.size()) ];
  //       InsertBirth( *(popM[repro_id]), repro_id, 1 );
  //
  //       // Reduce the probability of this organism reproducing again.
  //       repro_prob[repro_id] *= repro_decay;
  //     }
  //   }
  //
  };


}

#endif
