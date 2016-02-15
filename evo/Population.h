//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a Population template for use in evolutionary algorithms.
//
//  MEMBER can be any object type with the following properties:
//  * The copy constructor must work.
//
//  Dervied version of this class (with more complex population structuress)
//  may have additional requirements.


#ifndef EMP_POPULATION_H
#define EMP_POPULATION_H

#include <functional>
#include <map>

#include "../tools/assert.h"
#include "../tools/Random.h"
#include "../tools/vector.h"

namespace emp {
namespace evo {

  template <typename MEMBER>
  class Population {
  protected:
    emp::vector<MEMBER *> pop;
    emp::vector<MEMBER *> next_pop;
    std::function<double(MEMBER*)> default_fit_fun;
 
  public:
    Population() { ; }
    Population(const Population &) = default;
    ~Population() { Clear(); }
    Population & operator=(const Population &) = default;

    int GetSize() const { return (int) pop.size(); }
    const std::function<double(MEMBER*)> & GetDefaultFitnessFun() const { return default_fit_fun; }
    MEMBER & operator[](int i) { return *(pop[i]); }

    void SetDefaultFitnessFun(const std::function<double(MEMBER*)> & f) {
      default_fit_fun = f;
    }
    
    
    void Clear() {
      // Clear out all organisms.
      for (MEMBER * m : pop) delete m;
      for (MEMBER * m : next_pop) delete m;

      pop.resize(0);
      next_pop.resize(0);
    }
    
    void Insert(const MEMBER & mem, int copy_count=1) {
      for (int i = 0; i < copy_count; i++) pop.push_back(new MEMBER(mem));
    }
    void InsertNext(const MEMBER & mem, int copy_count=1) {
      for (int i = 0; i < copy_count; i++) next_pop.push_back(new MEMBER(mem));
    }

    // Selection mechanisms choose organisms for the next generation.

    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(std::function<double(MEMBER*)> fit_fun, int e_count=1, int copy_count=1) {
      emp_assert(e_count > 0 && e_count <= (int) pop.size());

      // Load the population into a multimap, sorted by fitness.
      std::multimap<double, int> fit_map;
      for (int i = 0; i < (int) pop.size(); i++) {
        fit_map.insert( std::make_pair(fit_fun(pop[i]), i) );
      }

      // Grab the top fitnesses and move them into the next generation.
      auto m = fit_map.rbegin();
      for (int i = 0; i < e_count; i++) {
        InsertNext( *(pop[m->second]), copy_count);
        ++m;
      }
    }

    // Elite Selection can use the default fitness function.
    void EliteSelect(int e_count=1, int copy_count=1) {
      EliteSelect(default_fit_fun, e_count, copy_count);
    }

    // Tournament Selection create a tournament with a random sub-set of organisms,
    // finds the one with the highest fitness, and moves it to the next generation.
    // User provides the fitness function, the tournament size, the random-number generator
    // and (optionally) the number of tournaments to run.
    void TournamentSelect(std::function<double(MEMBER*)> fit_fun, int t_size,
                          Random & random, int tourny_count=1) {
      emp_assert(t_size > 0 && t_size <= (int) pop.size());

      // Pre-calculate fitnesses.
      std::vector<double> fitness(pop.size());
      for (int i = 0; i < (int) pop.size(); ++i) fitness[i] = fit_fun(pop[i]);

      RunTournament(fitness, t_size, random, tourny_count);

    }

    // Tournament Selection can use the default fitness function.
    void TournamentSelect(int t_size, Random & random, int tourny_count=1) {
      TournamentSelect(default_fit_fun, t_size, random, tourny_count);
    }

    // Helper function to actually run tournament
    void RunTournament(std::vector<double> fitness, int t_size, 
		       Random & random, int tourny_count=1){
      for (int T = 0; T < tourny_count; T++) {
        std::vector<int> entries = random.Choose(pop.size(), t_size);
        double best_fit = fitness[entries[0]];
        int best_id = entries[0];
      
        // Search for a higher fit org in the tournament.
        for (int i = 1; i < t_size; i++) {
          const double cur_fit = fitness[entries[i]];
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = entries[i];
          }
        }
        
        // Place the highest fitness into the next generation!
        InsertNext( *(pop[best_id]) );
      }
    }

    // Run tournament selection with fitnesses adjusted by Goldberg and
    // Richardson's fitness sharing function (1987)
    // Requires a distance function that is valid for members of the population,
    // a sharing threshold (sigma share) that defines which members are
    // in the same niche, and a value of alpha (which controls the shape of
    // the fitness sharing curve
    void FitnessSharingTournamentSelect(std::function<double(MEMBER*)> 
					fit_fun, 
					std::function<double(MEMBER*, MEMBER*)>
					dist_fun,
					double sharing_threshhold, double alpha,
					int t_size, Random & random, 
					int tourny_count=1){

      emp_assert(t_size > 0 && t_size <= (int) pop.size());

      // Pre-calculate fitnesses.
      std::vector<double> fitness(pop.size());
      for (int i = 0; i < (int) pop.size(); ++i) {
	double niche_count = 0;
	for (int j = 0; j < (int) pop.size(); ++j) {
	  double dij = dist_fun(pop[i], pop[j]);
	  niche_count += std::max(1 - std::pow(dij/sharing_threshhold, alpha), 0.0);
	}
	fitness[i] = fit_fun(pop[i])/niche_count;
      }

      RunTournament(fitness, t_size, random, tourny_count);      
    }

    // Fitness sharing Tournament Selection can use the default fitness function
    void FitnessSharingTournamentSelect(std::function<double(MEMBER*, MEMBER*)>
					dist_fun, double sharing_threshold, 
					double alpha, int t_size, 
					Random & random, 
					int tourny_count=1) {
      TournamentSelect(default_fit_fun, dist_fun, sharing_threshold, alpha, 
		       t_size, random, tourny_count);
    }

    // Update() moves the next population to the current position, managing memory as needed.
    void Update() {
      for (MEMBER * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
    }
    
  };

}  // END evo namespace
}  // END emp namespace

#endif
