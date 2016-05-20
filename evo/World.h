//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines a World template for use in evolutionary algorithms.
//
//  The template *must* define ORG to identify the type of organism in the population.
//  The template *config* also be passed in any number of MANAGERS.
//
//  ORG can be any object type with the following properties:
//  * The copy constructor must work.
//
//  MANAGERS handle specific aspects of how the world should function, such as organism
//  placement in the population, systematics tracking, environmental resources, etc.
//
//    PopulationManager.h defines:
//      PopulationManager_Base<ORG>            -- Continuous generations
//      PopulationManager_EA<ORG>              -- Synchronous generations
//      PopulationManager_SerialTransfer<ORG>  -- Sequential bottlenecks
//
//  SIGNALS that will be triggered associated with world can customize behaviors.  Note that
//  all signal names must be prefixed with the world name so that the correct world is used.
//
//      ::before-repro(int parent_position)   Trigger: Immediately prior to producing offspring
//      ::offspring-ready(ORG * offspring)    Trigger: Offspring about to enter population
//      ::inject-ready(ORG * new_organism)    Trigger: New org about to be added to population
//      ::org-placement(int org_position)     Trigger: Organism has been added to population
//
//  Organisms can also trigger signals to affect the world.
//
//      ::do-repro(int org_id)                Trigger to initiate reproduction
//      ::do-symbiont_repro(int host_id)      Trigger to initiate symbiont reproduction
//
// Constructors:
//  World(const std::string & world_name="emp::evo::World")
//  World(int random_seed, const std::string & world_name="emp::evo::World")
//  World(emp::Random & random, const std::string & world_name="emp::evo::World")
//     world_name is prepended before all signal names.
//     random is used as a random number generator.
//
// Accessors
//  int GetSize() const
//  ORG & operator[](int i)
//  const std::function<double(ORG*)> & GetDefaultFitnessFun() const
//  const std::function<bool(ORG*)> & GetDefaultMutationFun() const
//  void SetDefaultFitnessFun(const std::function<double(ORG*)> & f)
//  void SetDefaultMutationFun(const std::function<bool(ORG*)> & f)
//
// Population Building
//  void Clear()                                           - Clear all organisms in population
//  void Insert(const ORG & mem, int copy_count=1)      - Insert a copy of an individual
//  void Insert(Random & random, ARGS... args)             - Insert a random organism
//  void InsertRandomOrg(ARGS... args)
//  void InsertNext(const ORG & mem, int copy_count=1)  - Insert into NEXT GENERATION of pop
//
// Organism Variation
//  int MutatePop(std::function<bool(ORG*)> mut_fun, int first_mut=0, int last_mut=-1)
//  int MutatePop(int first_mut=0, int last_mut=-1)
//
// Selection Mechanisms
//  void EliteSelect(std::function<double(ORG*)> fit_fun, int e_count=1, int copy_count=1)
//  void EliteSelect(int e_count=1, int copy_count=1)
//
//  void TournamentSelect(std::function<double(ORG*)> fit_fun, int t_size, int tourny_count=1)
//  void TournamentSelect(int t_size, int tourny_count=1)
//  void RunTournament(emp::vector<double> fitness, int t_size, int tourny_count=1)
//
//  void FitnessSharingTournamentSelect(std::function<double(ORG*)> fit_fun,
//          std::function<double(ORG*, ORG*)> dist_fun,
//          double sharing_threshhold, double alpha,
//          int t_size, int tourny_count=1)
//  void FitnessSharingTournamentSelect(std::function<double(ORG*, ORG*)>
//          dist_fun, double sharing_threshold,
//          double alpha, int t_size,
//          int tourny_count=1)
//
// Advancing Time
//  void Update()                  -- Shift to the next generation.
//  void Execute(ARGS... args)     -- Execute each organism in the population.


#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>
#include <map>

#include "../tools/assert.h"
#include "../tools/Random.h"
#include "../tools/reflection.h"
#include "../tools/vector.h"

#include "OrgSignals.h"
#include "OrgManager.h"
#include "PopulationManager.h"
#include "WorldIterator.h"

// Macro to add class elements associated with a dynamic function call.
// For example, if you wanted to be able to have a dynamic fitness function, you would call:
//
//   EMP_SETUP_EVO_WORLD_DEFAULT(default_fit_fun, Fitness, double)
//
// This macro will create a function object called "default_fit_fun), which takes a pointer
// to a member and converts it into a double value representing fitness.
//
// It will also create member fuctions in the world:
//   Setup_Fitness()  -- call in Setup to make sure we use ORG.Fitness() if it exists.
//   GetDefaultFitnessFun()  -- Return the current default fitness function being used.
//   SetDefaultFitnessFun(new_fun)  -- Set the default fitness function to be new_fun.

#define EMP_EVO_FORWARD(FUN, TARGET) \
template <typename... T> void FUN(T &&... args) { TARGET.FUN(std::forward<T>(args)...); }

namespace emp {
namespace evo {

  EMP_SETUP_TYPE_SELECTOR(SelectPopManager, emp_is_population_manager);
  EMP_SETUP_TYPE_SELECTOR(SelectOrgManager, emp_is_organism_manager);

  template <typename ORG, typename... MANAGERS> class WorldIterator;

  // Main world class...
  template <typename ORG, typename... MANAGERS>
  class World {
  public:
    // Build managers...
    AdaptTemplate<typename SelectPopManager<MANAGERS...,PopBasic>::type, ORG> popM;
    AdaptTemplate<typename SelectOrgManager<MANAGERS...,OrgMDynamic>::type, ORG> orgM;

    Random * random_ptr;
    bool random_owner;
    int update = 0;
    friend class WorldIterator<ORG, MANAGERS...>;
    typedef WorldIterator<ORG, MANAGERS...> iterator;

    // Signals triggered by the world.
    Signal<int> before_repro_sig;       // Trigger: Immediately prior to producing offspring
    Signal<ORG *> offspring_ready_sig;  // Trigger: Offspring about to enter population
    Signal<ORG *> inject_ready_sig;        // Trigger: New org about to be added to population
    Signal<int> org_placement_sig;         // Trigger: Organism has been added to population
    Signal<int> on_update_sig;         // Trigger: Organism has been added to population

    // Determine the callback type; by default this will be OrgSignals_NONE, but it can be
    // overridden by setting the type callback_t in the organism class.
    EMP_CHOOSE_MEMBER_TYPE(callback_t, callback_t, OrgSignals_NONE, ORG);
    callback_t callbacks;

    // Build a Setup method in world that calls .Setup() on whatever is passed in, but
    // only if it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);


    void SetupCallbacks(OrgSignals_NONE &) { ; }
    void SetupCallbacks(OrgSignals_Basic & sigs) {
      sigs.repro_sig.AddAction([this](int id){DoRepro(id);});
    }
    void SetupCallbacks(OrgSignals_Eco & sigs) {
      sigs.repro_sig.AddAction([this](int id){DoRepro(id);});
      sigs.symbiont_repro_sig.AddAction([this](int id){DoSymbiontRepro(id);});
    }

    void SetupWorld() {
      SetupCallbacks(callbacks);
      popM.SetRandom(random_ptr);
    }

  public:
    World(emp::Random * r_ptr, const std::string & pop_name=GenerateSignalName("emp::evo::World"))
      : random_ptr(r_ptr), random_owner(false)
      , before_repro_sig(to_string(pop_name,"::before-repro"))
      , offspring_ready_sig(to_string(pop_name,"::offspring-ready"))
      , inject_ready_sig(to_string(pop_name,"::inject-ready"))
      , org_placement_sig(to_string(pop_name,"::org-placement"))
      , on_update_sig(to_string(pop_name,"::on-update"))
      , callbacks(pop_name) { SetupWorld(); }

    World(int seed=-1, const std::string & pop_name=GenerateSignalName("emp::evo::World"))
      : World(new Random(seed), pop_name) { random_owner = true; }
    World(emp::Random & random, const std::string & pop_name=GenerateSignalName("emp::evo::World"))
      : World(&random, pop_name) { ; }
    World(const World &) = delete;
    ~World() { Clear(); if (random_owner) delete random_ptr; }
    World & operator=(const World &) = delete;

    int GetSize() const { return (int) popM.size(); }
    ORG & operator[](int i) { return *(popM[i]); }
    const ORG & operator[](int i) const { return *(popM[i]); }
    bool IsOccupied(int i) const { return popM[i] != nullptr; }
    iterator begin(){return WorldIterator<ORG, MANAGERS...>(this, 0);}
    iterator end(){return WorldIterator<ORG, MANAGERS...>(this, this->GetSize());}

    void Clear() { popM.Clear(); }

    Random & GetRandom() { return *random_ptr; }
    void SetRandom(Random & random) { if (random_owner) delete random_ptr; random_ptr = &random; }
    void ResetRandom(int seed=-1) { SetRandom(*(new Random(seed))); }

    // Forward function calls to appropriate internal objects
    EMP_EVO_FORWARD(ConfigPop, popM);
    EMP_EVO_FORWARD(SetDefaultFitnessFun, orgM);
    EMP_EVO_FORWARD(SetDefaultMutateFun, orgM);

    LinkKey OnBeforeRepro(std::function<void(int)> fun) { return before_repro_sig.AddAction(fun); }
    LinkKey OnOffspringReady(std::function<void(ORG *)> fun) { return offspring_ready_sig.AddAction(fun); }
    LinkKey OnInjectReady(std::function<void(ORG *)> fun) { return inject_ready_sig.AddAction(fun); }
    LinkKey OnOrgPlacement(std::function<void(int)> fun) { return org_placement_sig.AddAction(fun); }
    LinkKey OnUpdate(std::function<void(int)> fun) { return on_update_sig.AddAction(fun); }

    // All additions to the population must go through one of the following Insert methods

    void Insert(const ORG & mem, int copy_count=1) {
      for (int i = 0; i < copy_count; i++) {
        ORG * new_org = new ORG(mem);
        inject_ready_sig.Trigger(new_org);
        const int pos = popM.AddOrg(new_org);
        SetupOrg(*new_org, &callbacks, pos);
        org_placement_sig.Trigger(pos);
      }
    }
    template <typename... ARGS>
    void InsertRandomOrg(ARGS... args) {
      emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
      ORG * new_org = new ORG(*random_ptr, std::forward<ARGS>(args)...);
      inject_ready_sig.Trigger(new_org);
      const int pos = popM.AddOrg(new_org);
      SetupOrg(*new_org, &callbacks, pos);
      org_placement_sig.Trigger(pos);
    }
    void InsertBirth(const ORG & mem, int parent_pos, int copy_count=1) {
      before_repro_sig.Trigger(parent_pos);
      for (int i = 0; i < copy_count; i++) {
        ORG * new_org = new ORG(mem);
        offspring_ready_sig.Trigger(new_org);
        const int pos = popM.AddOrgBirth(new_org, parent_pos);
        SetupOrg(*new_org, &callbacks, pos);
        org_placement_sig.Trigger(pos);
      }
    }


    void DoRepro(int id) {
      emp_assert(random_ptr != nullptr && "DoRepro() requires a random number generator.");
      // std::cout << "Repro " << id << std::endl;
      before_repro_sig.Trigger(id);
      InsertBirth(*(popM[id]), id, 1);

    }

    void DoSymbiontRepro(int id) {
      emp_assert(random_ptr != nullptr && "DoSymbiontRepro() requires a random number generator.");
      // std::cout << "SymbRepro " << id << std::endl;

      // @CAO For the moment, assume random replacement (in the future, make pop_manager handle it)
      const int target_id = random_ptr->GetInt((int) popM.size());

      // Copy the symbiont into the target.
      const auto & symbiont = popM[id]->GetSymbiont();
      popM[target_id]->InjectSymbiont(symbiont, *random_ptr);
    }

    // Mutations for the next generation (count number of mutated organisms)
    int MutatePop(std::function<bool(ORG*,emp::Random&)> mut_fun,
                  int first_mut=0, int last_mut=-1) {
      emp_assert(mut_fun);
      emp_assert(random_ptr != nullptr && "Mutate() requires active random_ptr");
      if (last_mut == -1) last_mut = (int) popM.size();
      int mut_count = 0;
      for (int i = first_mut; i < last_mut; i++) {
        if (mut_fun(popM[i], *random_ptr)) mut_count++;
      }
      return mut_count;
    }

    int MutatePop(const int first_mut=0, const int last_mut=-1) {
      return MutatePop(orgM.GetMutFun(), first_mut, last_mut);
    }

    void Print(std::ostream & os=std::cout, const std::string & empty="X", const std::string & spacer=" ") {
      popM.Print(os, empty, spacer);
    }

    //Helper function to return PopulationManager indices of
    //all organisms that are not null
    emp::vector<int> GetValidOrgIndices(){
      emp::vector<int> valid_orgs(0);
      for (int i = 0; i < popM.size(); i++){
        if (this->IsOccupied(i)){
          valid_orgs.push_back(i);
        }
      }
      return valid_orgs;
    }

    // Selection mechanisms choose organisms for the next generation.

    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(std::function<double(ORG*)> fit_fun, int e_count=1, int copy_count=1) {
      emp_assert(fit_fun);
      emp_assert(e_count > 0 && e_count <= (int) popM.size());

      // Load the population into a multimap, sorted by fitness.
      std::multimap<double, int> fit_map;
      for (int i = 0; i < (int) popM.size(); i++) {
        fit_map.insert( std::make_pair(fit_fun(popM[i]), i) );
      }

      // Grab the top fitnesses and move them into the next generation.
      auto m = fit_map.rbegin();
      for (int i = 0; i < e_count; i++) {
        InsertBirth( *(popM[m->second]), m->second, copy_count);
        ++m;
      }
    }

    // Elite Selection can use the default fitness function.
    void EliteSelect(int e_count=1, int copy_count=1) {
      EliteSelect(orgM.GetFitFun(), e_count, copy_count);
    }

    // Tournament Selection create a tournament with a random sub-set of organisms,
    // finds the one with the highest fitness, and moves it to the next generation.
    // User provides the fitness function, the tournament size, and (optionally) the
    // number of tournaments to run.
    void TournamentSelect(std::function<double(ORG*)> fit_fun, int t_size,
        int tourny_count=1, bool precalc_fitness=true) {
      emp_assert(fit_fun);
      emp_assert(t_size > 0 && t_size <= (int) popM.size(), t_size, popM.size());

      if (precalc_fitness && t_size * tourny_count * 2 >= (int) popM.size()) {
        // Pre-calculate fitnesses.
        emp::vector<double> fitness(popM.size());
        for (int i = 0; i < (int) popM.size(); ++i) fitness[i] = fit_fun(popM[i]);

        RunTournament(fitness, t_size, tourny_count);
      }
      else RunTournament(fit_fun, t_size, tourny_count);
    }

    // Tournament Selection can use the default fitness function.
    void TournamentSelect(int t_size, int tourny_count=1) {
      TournamentSelect(orgM.GetFitFun(), t_size, tourny_count);
    }

    // Helper function to run a tournament when fitness is pre-calculated
    void RunTournament(const emp::vector<double> & fitness, int t_size, int tourny_count=1){
      emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");

      emp::vector<int> valid_orgs = GetValidOrgIndices();

      for (int T = 0; T < tourny_count; T++) {
        emp::vector<int> entries = Choose(*random_ptr, valid_orgs.size(), t_size);
        double best_fit = fitness[valid_orgs[entries[0]]];
        int best_id = valid_orgs[entries[0]];

        // Search for a higher fit org in the tournament.
        for (int i = 1; i < t_size; i++) {
          const double cur_fit = fitness[valid_orgs[entries[i]]];
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = valid_orgs[entries[i]];
          }
        }

        // Place the highest fitness into the next generation!
        InsertBirth( *(popM[best_id]), best_id, 1 );
      }
    }

    // Helper function to run a tournament when fitness is NOT pre-calculated
    void RunTournament(std::function<double(ORG*)> fit_fun, int t_size, int tourny_count=1){
      emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");

      //This technique for avoiding null orgs can probably be improved
      //once org managers are handling that
      emp::vector<int> valid_orgs = GetValidOrgIndices();

      for (int T = 0; T < tourny_count; T++) {
        emp::vector<int> entries = Choose(*random_ptr, valid_orgs.size(), t_size);
        double best_fit = fit_fun(popM[valid_orgs[entries[0]]]);
        int best_id = valid_orgs[entries[0]];

        // Search for a higher fit org in the tournament.
        for (int i = 1; i < t_size; i++) {
          const double cur_fit = fit_fun(popM[valid_orgs[entries[i]]]);
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = valid_orgs[entries[i]];
          }
        }

        // Place the highest fitness into the next generation!
        InsertBirth( *(popM[best_id]), best_id, 1 );
      }
    }


    // Run tournament selection with fitnesses adjusted by Goldberg and
    // Richardson's fitness sharing function (1987)
    // Requires a distance function that is valid for members of the population,
    // a sharing threshold (sigma share) that defines which members are
    // in the same niche, and a value of alpha (which controls the shape of
    // the fitness sharing curve
    void FitnessSharingTournamentSelect(std::function<double(ORG*)> fit_fun,
          std::function<double(ORG*, ORG*)> dist_fun,
          double sharing_threshhold, double alpha,
          int t_size, int tourny_count=1)
    {
      emp_assert(t_size > 0 && t_size <= (int) popM.size());

      // Pre-calculate fitnesses.
      emp::vector<double> fitness(popM.size());
      for (int i = 0; i < (int) popM.size(); ++i) {
        double niche_count = 0;
        for (int j = 0; j < (int) popM.size(); ++j) {
          double dij = dist_fun(popM[i], popM[j]);
          niche_count += std::max(1 - std::pow(dij/sharing_threshhold, alpha), 0.0);
        }
        fitness[i] = fit_fun(popM[i])/niche_count;
      }

      RunTournament(fitness, t_size, tourny_count);
    }

    // Fitness sharing Tournament Selection can use the default fitness function
    void FitnessSharingTournamentSelect(std::function<double(ORG*, ORG*)>
          dist_fun, double sharing_threshold,
          double alpha, int t_size,
          int tourny_count=1) {
      TournamentSelect(orgM.GetFitFun(), dist_fun, sharing_threshold, alpha, t_size, tourny_count);
    }


    // Update() moves the next population to the current position, managing memory as needed.
    void Update() {
      on_update_sig.Trigger(update);
      update++;
      popM.Update();
    }


    // Execute() runs the Execute() method on all organisms in the population, forwarding
    // any arguments.
    template <typename... ARGS>
    void Execute(ARGS... args) {
      popM.Execute(std::forward<ARGS>(args)...);
    }

  };

  // Setup a pre-build EAWorld that automatically uses synchronous generations.
  template <typename ORG, typename... MANAGERS>
  using EAWorld = World<ORG, MANAGERS..., PopulationManager_EA<ORG>>;

  template <typename ORG, typename... MANAGERS>
  using GridWorld = World<ORG, MANAGERS..., PopulationManager_Grid<ORG>>;

}  // END evo namespace
}  // END emp namespace

#endif
