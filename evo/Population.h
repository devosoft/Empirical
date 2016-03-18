//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a Population template for use in evolutionary algorithms.
//
//  MEMBER can be any object type with the following properties:
//  * The copy constructor must work.
//
//  Dervied versions of this class (with more complex population structuress)
//  may have additional requirements.
//
// Constructors:
//  Population(const std::string & pop_name="emp::evo::Population")
//  Population(emp::Random & random, const std::string & pop_name="emp::evo::Population")
//     pop_name is prepended before all signal names.
//     random is used as a random number seed, if needed.
//
// Accessors
//  int GetSize() const
//  MEMBER & operator[](int i)
//  const std::function<double(MEMBER*)> & GetDefaultFitnessFun() const
//  const std::function<bool(MEMBER*)> & GetDefaultMutationFun() const
//  void SetDefaultFitnessFun(const std::function<double(MEMBER*)> & f)
//  void SetDefaultMutationFun(const std::function<bool(MEMBER*)> & f)
//
// Population Building
//  void Clear()                                           - Clear all organisms in population
//  void Insert(const MEMBER & mem, int copy_count=1)      - Insert a copy of an individual
//  void Insert(Random & random, ARGS... args)             - Insert a random organism
//  void InsertRandomOrg(ARGS... args)
//  void InsertNext(const MEMBER & mem, int copy_count=1)  - Insert into NEXT GENERATION of pop
//
// Population Variation
//  int Mutate(std::function<bool(MEMBER*)> mut_fun, const int first_mut=1)
//  int Mutate(const int first_mut=1)
//
// Selection Mechanisms
//  void EliteSelect(std::function<double(MEMBER*)> fit_fun, int e_count=1, int copy_count=1)
//  void EliteSelect(int e_count=1, int copy_count=1)
//
//  void TournamentSelect(std::function<double(MEMBER*)> fit_fun, int t_size, int tourny_count=1)
//  void TournamentSelect(int t_size, int tourny_count=1)
//  void RunTournament(std::vector<double> fitness, int t_size, int tourny_count=1)
//
//  void FitnessSharingTournamentSelect(std::function<double(MEMBER*)> fit_fun, 
//					std::function<double(MEMBER*, MEMBER*)> dist_fun,
//					double sharing_threshhold, double alpha,
//					int t_size, int tourny_count=1)
//  void FitnessSharingTournamentSelect(std::function<double(MEMBER*, MEMBER*)>
//					dist_fun, double sharing_threshold, 
//					double alpha, int t_size, 
//					int tourny_count=1)
//
// Advancing Population
//  void Update()                  -- Shift to the next generation.
//  void Execute(ARGS... args)     -- Execute each organism in the population.

	

#ifndef EMP_POPULATION_H
#define EMP_POPULATION_H

#include <functional>
#include <map>

#include "../tools/assert.h"
#include "../tools/Random.h"
#include "../tools/reflection.h"
#include "../tools/vector.h"

#include "OrgSignals.h"

namespace emp {
namespace evo {

#define EMP_SETUP_EVO_POP_DEFAULT(TYPE, FUN_VAR, METHOD)			\
  TYPE FUN_VAR;								\
  template <class T> void Setup_ ## METHOD ## _impl(emp_bool_decoy(T::METHOD)) { \
    FUN_VAR = [](T* org){ return org->GetMETHOD(); };			\
  }									\
  template <class T> void Setup_ ## METHOD ## _impl(int) { ; }		\
  void Setup_ ## METHOD() { Setup_ ## METHOD ## _impl<MEMBER>(true); }


  // Main population class...
  
  template <typename MEMBER>
  class Population {
  protected:
    emp::vector<MEMBER *> pop;
    emp::vector<MEMBER *> next_pop;
    emp::Random * random_ptr;
    EMP_SETUP_EVO_POP_DEFAULT(std::function<double(MEMBER*)>, default_fit_fun, Fitness);
    EMP_SETUP_EVO_POP_DEFAULT(std::function<bool(MEMBER*)>, default_mut_fun, Mutate);

    // Determine the callback type; by default this will be OrgSignals_NONE, but it can be
    // overridden by setting the type callback_t in the organism class.
    EMP_CREATE_TYPE_FALLBACK(callback_t, MEMBER, callback_t, OrgSignals_NONE);
    callback_t callbacks;

    // Build a Setup method in population that calls .Setup() on whatever is passed in, but
    // only if it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);

    
    // AddOrg and ReplaceOrg should be the only ways new organisms come into a population.
    // AddOrg inserts them into the end of the designated population.
    // ReplaceOrg places them at a specific position, replacing anyone who may already be there.
    void AddOrg(emp::vector<MEMBER *> & target_pop, MEMBER * new_org) {
      SetupOrg(*new_org, &callbacks, (int) target_pop.size());
      target_pop.push_back(new_org);
    }

    void ReplaceOrg(emp::vector<MEMBER *> & target_pop, int pos, MEMBER * new_org) {
      if (pop[pos]) delete pop[pos];
      pop[pos] = new_org;
      SetupOrg(*new_org, &callbacks, pos);
    }
    
    void DoRepro(int id) {
      emp_assert(random_ptr != nullptr && "DoRepro() requires a random number generator.");
      std::cout << "Repro " << id << std::endl;
      MEMBER * new_org = new MEMBER(*(pop[id]));
      // @CAO For the moment, assume random replacement (in the future, setup a topology)
      const int target_id = random_ptr->GetInt((int) pop.size());
      ReplaceOrg(pop, target_id, new_org);
    }
    
    void DoSymbiontRepro(int id) {
      emp_assert(random_ptr != nullptr && "DoSymbiontRepro() requires a random number generator.");
      std::cout << "SymbRepro " << id << std::endl;
      // @CAO For the moment, assume random replacement (in the future, setup a topology)
      const int target_id = random_ptr->GetInt((int) pop.size());

      // @CAO Call member class function to determine which symboiont is being comied from
      // the host in id, and how (and if) they should going into the target id.
    }
    
    void SetupCallbacks(OrgSignals_NONE &) { ; }
    void SetupCallbacks(OrgSignals_Basic & sigs) {
      sigs.repro_sig.AddAction([this](int id){DoRepro(id);});
    }
    void SetupCallbacks(OrgSignals_Eco & sigs) {
      sigs.repro_sig.AddAction([this](int id){DoRepro(id);});
      sigs.symbiont_repro_sig.AddAction([this](int id){DoSymbiontRepro(id);});
    }
    
  public:
    Population(const std::string & pop_name="emp::evo::Population")
      : random_ptr(nullptr), callbacks(pop_name)
    {
      SetupCallbacks(callbacks);
      Setup_Fitness();
    }
    Population(emp::Random & random, const std::string & pop_name="emp::evo::Population")
      : Population(pop_name) { random_ptr = &random; }
    Population(const Population &) = delete;
    ~Population() { Clear(); }
    Population & operator=(const Population &) = delete;

    int GetSize() const { return (int) pop.size(); }
    const std::function<double(MEMBER*)> & GetDefaultFitnessFun() const { return default_fit_fun; }
    const std::function<bool(MEMBER*)> & GetDefaultMutationFun() const { return default_mut_fun; }
    MEMBER & operator[](int i) { return *(pop[i]); }

    void SetDefaultFitnessFun(const std::function<double(MEMBER*)> & f) {
      default_fit_fun = f;
    }
    void SetDefaultMutationFun(const std::function<bool(MEMBER*)> & f) {
      default_mut_fun = f;
    }
    
    
    void Clear() {
      // Clear out all organisms.
      for (MEMBER * m : pop) delete m;
      for (MEMBER * m : next_pop) delete m;

      pop.resize(0);
      next_pop.resize(0);
    }

    // All additions to the population must go through one of the following Insert methods
    // (all of which call AddOrg())

    void Insert(const MEMBER & mem, int copy_count=1) {
      for (int i = 0; i < copy_count; i++) AddOrg(pop, new MEMBER(mem));
    }
    template <typename... ARGS>
    void Insert(Random & random, ARGS... args) {
      AddOrg(pop, new MEMBER(random, std::forward<ARGS>(args)...));      
    }
    template <typename... ARGS>
    void InsertRandomOrg(ARGS... args) {
      emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
      AddOrg(pop, new MEMBER(*random_ptr, std::forward<ARGS>(args)...));      
    }
    void InsertNext(const MEMBER & mem, int copy_count=1) {
      for (int i = 0; i < copy_count; i++) AddOrg(next_pop, new MEMBER(mem));
    }

    // Mutations for the next generation (count number of mutated organisms)
    int Mutate(std::function<bool(MEMBER*)> mut_fun, const int first_mut=1) {
      int mut_count = 0;
      for (int i = 1; i < (int) pop.size(); i++) {
	if (mut_fun(pop[i])) mut_count++;
      }
      return mut_count;
    }

    int Mutate(const int first_mut=1) {
      return Mutate(default_mut_fun, first_mut);
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
    // User provides the fitness function, the tournament size, and (optionally) the
    // number of tournaments to run.
    void TournamentSelect(std::function<double(MEMBER*)> fit_fun, int t_size,
			  int tourny_count=1) {
      emp_assert(t_size > 0 && t_size <= (int) pop.size());

      // Pre-calculate fitnesses.
      std::vector<double> fitness(pop.size());
      for (int i = 0; i < (int) pop.size(); ++i) fitness[i] = fit_fun(pop[i]);

      RunTournament(fitness, t_size, tourny_count);
    }

    // Tournament Selection can use the default fitness function.
    void TournamentSelect(int t_size, int tourny_count=1) {
      TournamentSelect(default_fit_fun, t_size, tourny_count);
    }

    // Helper function to actually run a tournament
    void RunTournament(std::vector<double> fitness, int t_size, int tourny_count=1){
      emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");
      for (int T = 0; T < tourny_count; T++) {
        std::vector<int> entries = random_ptr->Choose(pop.size(), t_size);
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
    void FitnessSharingTournamentSelect(std::function<double(MEMBER*)> fit_fun, 
					std::function<double(MEMBER*, MEMBER*)> dist_fun,
					double sharing_threshhold, double alpha,
					int t_size, int tourny_count=1){

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

      RunTournament(fitness, t_size, tourny_count);      
    }

    // Fitness sharing Tournament Selection can use the default fitness function
    void FitnessSharingTournamentSelect(std::function<double(MEMBER*, MEMBER*)>
					dist_fun, double sharing_threshold, 
					double alpha, int t_size, 
					int tourny_count=1) {
      TournamentSelect(default_fit_fun, dist_fun, sharing_threshold, alpha, t_size, tourny_count);
    }


    // Update() moves the next population to the current position, managing memory as needed.
    void Update() {
      for (MEMBER * m : pop) delete m;   // Delete the current population.
      pop = next_pop;                    // Move over the next generation.
      next_pop.resize(0);                // Clear out the next pop to refill again.
    }


    // Execute() runs the Execute() method on all organisms in the population, forwarding
    // any arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      for (MEMBER * m : pop) {
	m->Execute(std::forward<ARGS>(args)...);
      }
    }
    
  };

}  // END evo namespace
}  // END emp namespace

#endif
