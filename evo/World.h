//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
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
//      ::before-repro(size_t parent_position) Trigger: Immediately prior to producing offspring
//      ::offspring-ready(ORG * offspring)     Trigger: Offspring about to enter population
//      ::inject-ready(ORG * new_organism)     Trigger: New org about to be added to population
//      ::org-placement(size_t org_position)   Trigger: Organism has been added to population
//
//  Organisms can also trigger signals to affect the world.
//
//      ::do-repro(size_t org_id)              Trigger to initiate reproduction
//      ::do-symbiont_repro(size_t host_id)    Trigger to initiate symbiont reproduction
//
//  Constructors:
//   World(const std::string & world_name="emp::evo::World")
//   World(int random_seed, const std::string & world_name="emp::evo::World")
//   World(emp::Random & random, const std::string & world_name="emp::evo::World")
//      world_name is prepended before all signal names.
//      random is used as a random number generator.
//
//  Accessors
//   size_t GetSize() const
//   ORG & operator[](size_t i)
//   const std::function<double(ORG*)> & GetDefaultFitnessFun() const
//   const std::function<bool(ORG*)> & GetDefaultMutationFun() const
//   void SetDefaultFitnessFun(const std::function<double(ORG*)> & f)
//   void SetDefaultMutationFun(const std::function<bool(ORG*)> & f)
//
//  Population Building
//   void Clear()                                        - Clear all organisms in population
//   void Insert(const ORG & mem, size_t copy_count=1)   - Insert a copy of an individual
//   void InsertAt(const ORG & mem, size_t pos)          - Insert an individual at specified pos
//   void Insert(Random & random, ARGS... args)          - Insert a random organism
//   void InsertRandomOrg(ARGS... args)
//   void InsertNext(const ORG & mem, size_t copy_count=1) - Insert into NEXT GENERATION of pop
//
//  Organism Variation
//   int MutatePop(std::function<bool(ORG*)> mut_fun, int first_mut=0, int last_mut=-1)
//   int MutatePop(int first_mut=0, int last_mut=-1)
//
//  Selection Mechanisms
//   void EliteSelect(std::function<double(ORG*)> fit_fun, int e_count=1, int copy_count=1)
//   void EliteSelect(int e_count=1, int copy_count=1)
//
//   void TournamentSelect(std::function<double(ORG*)> fit_fun, int t_size, int tourny_count=1)
//   void TournamentSelect(int t_size, int tourny_count=1)
//   void RunTournament(emp::vector<double> fitness, int t_size, int tourny_count=1)
//
//   void FitnessSharingTournamentSelect(std::function<double(ORG*)> fit_fun,
//           std::function<double(ORG*, ORG*)> dist_fun,
//           double sharing_threshhold, double alpha,
//           int t_size, int tourny_count=1)
//   void FitnessSharingTournamentSelect(std::function<double(ORG*, ORG*)>
//           dist_fun, double sharing_threshold,
//           double alpha, int t_size,
//           int tourny_count=1)
//
//  Advancing Time
//   void Update()                  -- Shift to the next generation.
//   void Execute(ARGS... args)     -- Execute each organism in the population.


#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>
#include <map>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../control/SignalControl.h"
#include "../meta/reflection.h"
#include "../tools/Random.h"
#include "../tools/random_utils.h"

#include "FitnessManager.h"
#include "OrgSignals.h"
#include "OrgManager.h"
#include "PopulationManager.h"
#include "StatsManager.h"
#include "LineageTracker.h"


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


// @CAO Setup to dynamically choose correct version based on arg count.

#define EMP_EVO_FORWARD(FUN, TARGET) \
template <typename... T> void FUN(T &&... args) { TARGET.FUN(std::forward<T>(args)...); }

#define EMP_EVO_FORWARD_2(FUN, TARGET1, TARGET2)  \
template <typename... T> void FUN(T &&... args) { \
    TARGET1.FUN(std::forward<T>(args)...);        \
    TARGET2.FUN(std::forward<T>(args)...);        \
}

#define EMP_EVO_FORWARD_3(FUN, TARGET1, TARGET2, TARGET3) \
template <typename... T> void FUN(T &&... args) {         \
    TARGET1.FUN(std::forward<T>(args)...);                \
    TARGET2.FUN(std::forward<T>(args)...);                \
    TARGET3.FUN(std::forward<T>(args)...);                \
}

namespace emp {
namespace evo {

  EMP_SETUP_TYPE_SELECTOR(SelectFitnessManager, emp_is_fitness_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectLineageManager, emp_is_lineage_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectOrgManager, emp_is_organism_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectPopManager, emp_is_population_manager)
  EMP_SETUP_TYPE_SELECTOR(SelectStatsManager, emp_is_stats_manager)

  // Main world class...
  template <typename ORG, typename... MANAGERS>
  class World {
  public:
    // Some useful types...
    using fit_fun_t = std::function<double(ORG*)>;
    using dist_fun_t = std::function<double(ORG*,ORG*)>;

    // Determine manager types...
    using fitM_t = SelectFitnessManager<MANAGERS...,FitDynamic>;
    using popM_t = AdaptTemplate<SelectPopManager<MANAGERS...,PopBasic>, ORG, fitM_t>;
    using orgM_t = AdaptTemplate<SelectOrgManager<MANAGERS...,OrgMDynamic>, ORG>;
    using statsM_t = AdaptTemplate<SelectStatsManager<MANAGERS...,NullStats >, popM_t>;
    using iterator_t = PopulationIterator<popM_t>;

    //Create a lineage manager if the stats manager needs it or if the user asked for it
    EMP_CHOOSE_MEMBER_TYPE(DefaultLineage, lineage_type, LineageNull, statsM_t)
    using lineageM_t = AdaptTemplate<SelectLineageManager<MANAGERS...,DefaultLineage>, popM_t>;

    // Now that we've determined all of the manager types, build them!
    fitM_t fitM;
    popM_t popM;
    orgM_t orgM;
    statsM_t statsM;
    lineageM_t lineageM;

    Ptr<Random> random_ptr;
    bool random_owner;
    size_t update;

    // We have a set of signals that are triggered by the world.
    SignalControl control;

    // Trigger:  Immediately prior to parent producing offspring
    // Argument: Parent position in population
    Signal<void(size_t)> before_repro_sig;

    // Trigger: Offspring about to enter population
    // Argument: Pointer to organism about to be placed in population.
    Signal<void(ORG *)> offspring_ready_sig;

    // Trigger: New org about to be added to population
    // Argument: Pointer to organism about to be placed in population.
    Signal<void(ORG *)> inject_ready_sig;

    // Trigger: Organism has been added to population
    // Argument: Position of organism placed in the population.
    Signal<void(size_t)> org_placement_sig;

    // Trigger: New update is starting.
    // Argument: Update number (sequentially increasing)
    Signal<void(size_t)> on_update_sig;

    // Trigger: Location of org about to be killed
    // Argument: Pointer to organism about to die
    Signal<void(int)> on_death_sig;


    // Determine the callback type; by default this will be OrgSignals_NONE, but it can be
    // overridden by setting the type callback_t in the organism class.
    EMP_CHOOSE_MEMBER_TYPE(callback_t, callback_t, OrgSignals_NONE, ORG)
    callback_t callbacks;

    // Build a Setup method in world that calls ::Setup() on whatever is passed in if it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);


    void SetupCallbacks(OrgSignals_NONE &) { ; }
    void SetupCallbacks(OrgSignals_Basic & sigs) {
      sigs.repro_sig.AddAction([this](size_t id){DoRepro(id);});
    }
    void SetupCallbacks(OrgSignals_Eco & sigs) {
      sigs.repro_sig.AddAction([this](size_t id){DoRepro(id);});
      sigs.symbiont_repro_sig.AddAction([this](size_t id){DoSymbiontRepro(id);});
    }

  public:
    std::string world_name;

    World(Ptr<Random> r_ptr, const std::string & w_name=UniqueName("emp::evo::World"))
      : fitM(), popM(w_name, fitM), orgM(), statsM(), lineageM()
      , random_ptr(r_ptr), random_owner(false)
      , update(0), control()
      , before_repro_sig(to_string(w_name,"::before-repro"), control)
      , offspring_ready_sig(to_string(w_name,"::offspring-ready"), control)
      , inject_ready_sig(to_string(w_name,"::inject-ready"), control)
      , org_placement_sig(to_string(w_name,"::org-placement"), control)
      , on_update_sig(to_string(w_name,"::on-update"), control)
      , on_death_sig(to_string(w_name,"::on-death"), control)
      , callbacks(w_name)
      , world_name(w_name)
    {
      SetupCallbacks(callbacks);
      lineageM.Setup(this);
      statsM.Setup(this);
      popM.Setup(random_ptr);
    }

    World(int seed=-1, const std::string & w_name=UniqueName("emp::evo::World"))
      : World(new Random(seed), w_name) { random_owner = true; }
    World(emp::Random & random, const std::string & w_name=UniqueName("emp::evo::World"))
      : World(&random, w_name) { ; }
    World(const World &) = delete;
    ~World() { Clear(); if (random_owner) random_ptr.Delete(); }
    World & operator=(const World &) = delete;

    size_t GetSize() const { return popM.GetSize(); }
    size_t GetNumOrgs() const { return popM.GetNumOrgs(); }
    ORG & operator[](size_t i) { return *(popM[i]); }
    const ORG & operator[](size_t i) const { return *(popM[i]); }
    ORG & GetRandomOrg() {
      emp_assert(random_ptr != nullptr && "GetRandomOrg() requires active random_ptr");
      const size_t org_id = popM.GetRandomOrg();
      return *(popM[org_id]);
    }
    bool IsOccupied(size_t i) const { return popM[i] != nullptr; }
    iterator_t begin(){ return iterator_t(&popM, 0); }
    iterator_t end(){ return iterator_t(&popM, popM.size()); }

    void Clear() { popM.Clear(); }

    Random & GetRandom() { return *random_ptr; }
    void SetRandom(Random & random) {
      if (random_owner) random_ptr.Delete();
      random_ptr = &random;
      random_owner = false;
    }
    void ResetRandom(int seed=-1) { SetRandom(*(new Random(seed))); random_owner = true; }

    // Forward function calls to appropriate internal objects
    EMP_EVO_FORWARD(ConfigPop, popM)
    EMP_EVO_FORWARD_2(SetDefaultFitnessFun, orgM, statsM)
    EMP_EVO_FORWARD(SetDefaultMutateFun, orgM)

    // Deal with Signals
    SignalKey OnBeforeRepro(const std::function<void(size_t)> & fun) { return before_repro_sig.AddAction(fun); }
    SignalKey OnOffspringReady(const std::function<void(ORG *)> & fun) { return offspring_ready_sig.AddAction(fun); }
    SignalKey OnInjectReady(const std::function<void(ORG *)> & fun) { return inject_ready_sig.AddAction(fun); }
    SignalKey OnOrgPlacement(const std::function<void(size_t)> & fun) { return org_placement_sig.AddAction(fun); }
    SignalKey OnUpdate(const std::function<void(size_t)> & fun) { return on_update_sig.AddAction(fun); }
    SignalKey OnOrgDeath(const std::function<void(int)> & fun) { return on_death_sig.AddAction(fun); }

    // Forward any link requests to the SignalControl object.
    template <typename... ARGS>
    SignalKey Link(ARGS &&... args) { return control.Link(std::forward<ARGS>(args)...); }

    SignalControl & GetSignalControl() { return control; }

    fit_fun_t GetFitFun() { return orgM.GetFitFun(); }

    // All additions to the population must go through one of the following Insert methods

    void Insert(const ORG & mem, size_t copy_count=1) {
      for (size_t i = 0; i < copy_count; i++) {
        Ptr<ORG> new_org;
        new_org.New(mem);
        inject_ready_sig.Trigger(new_org.Raw());
        const size_t pos = popM.AddOrg(new_org);
        SetupOrg(*new_org, &callbacks, pos);
        org_placement_sig.Trigger(pos);
      }
    }

    void InsertAt(const ORG & mem, const size_t pos) {
      Ptr<ORG> new_org = NewPtr<ORG>(mem);
      inject_ready_sig.Trigger(new_org.Raw());
      popM.AddOrgAt(new_org, pos);
      SetupOrg(*new_org, &callbacks, pos);
      org_placement_sig.Trigger(pos);
    }

    template <typename... ARGS>
    void InsertRandomOrg(ARGS &&... args) {
      emp_assert(random_ptr != nullptr && "InsertRandomOrg() requires active random_ptr");
      Ptr<ORG> new_org = NewPtr<ORG>(*random_ptr, std::forward<ARGS>(args)...);
      inject_ready_sig.Trigger(new_org.Raw());
      const size_t pos = popM.AddOrg(new_org);
      SetupOrg(*new_org, &callbacks, pos);
      org_placement_sig.Trigger(pos);
    }

    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count=1) {
      before_repro_sig.Trigger(parent_pos);
      for (size_t i = 0; i < copy_count; i++) {
        Ptr<ORG> new_org = NewPtr<ORG>(mem);
        offspring_ready_sig.Trigger(new_org.Raw());
        const size_t pos = popM.AddOrgBirth(new_org, parent_pos);
        SetupOrg(*new_org, &callbacks, pos);
        org_placement_sig.Trigger(pos);
      }
    }

    // If InsertBirth is provided with a fitness function, use it to calculate fitness of new org.
    void InsertBirth(const ORG mem, size_t parent_pos, size_t copy_count,
                     const fit_fun_t & fit_fun) {
      before_repro_sig.Trigger(parent_pos);
      for (size_t i = 0; i < copy_count; i++) {
        Ptr<ORG> new_org;
        new_org.New(mem);
        offspring_ready_sig.Trigger(new_org.Raw());
        const size_t pos = popM.AddOrgBirth(new_org, parent_pos);
        // If we offspring are placed into the same population, recalculate fitness.
        if (popM_t::emp_has_separate_generations == false) popM.CalcFitness(pos, fit_fun);
        SetupOrg(*new_org, &callbacks, pos);
        org_placement_sig.Trigger(pos);
      }
    }

    void DoRepro(size_t id) {
      emp_assert(random_ptr != nullptr && "DoRepro() requires a random number generator.");
      before_repro_sig.Trigger(id);
      InsertBirth(*(popM[id]), id, 1);
    }

    void DoSymbiontRepro(size_t id) {
      emp_assert(random_ptr != nullptr && "DoSymbiontRepro() requires a random number generator.");
      // std::cout << "SymbRepro " << id << std::endl;

      // @CAO For the moment, assume random replacement (in the future, make pop_manager handle it)
      const size_t target_id = random_ptr->GetUInt(popM.size());

      // Copy the symbiont into the target.
      const auto & symbiont = popM[id]->GetSymbiont();
      popM[target_id]->InjectSymbiont(symbiont, *random_ptr);
    }

    // Mutations for the next generation (count number of mutated organisms)
    size_t MutatePop(const std::function<bool(ORG*,emp::Random&)> & mut_fun,
                     size_t first_mut, size_t last_mut) {
      emp_assert(mut_fun);
      emp_assert(random_ptr != nullptr && "Mutate() requires active random_ptr");
      size_t mut_count = 0;
      for (size_t i = first_mut; i < last_mut; i++) {
        if (this->IsOccupied(i)){
          if (mut_fun(popM[i], *random_ptr)) {
            mut_count++;
            fitM.ClearAt(i);
          }
        }
      }
      return mut_count;
    }

    size_t MutatePop(const std::function<bool(ORG*,emp::Random&)> & mut_fun, size_t first_mut=0) {
      size_t last_mut = popM.size();
      return MutatePop(mut_fun, first_mut, last_mut);
    }

    size_t MutatePop(const size_t first_mut, const size_t last_mut) {
      return MutatePop(orgM.GetMutFun(), first_mut, last_mut);
    }

    size_t MutatePop(const size_t first_mut=0) {
      return MutatePop(orgM.GetMutFun(), first_mut);
    }

      void Print(const std::function<std::string(ORG*)> & print_fun,
               std::ostream & os=std::cout,
               const std::string & empty="-",
               const std::string & spacer=" ") {
      popM.Print(print_fun, os, empty, spacer);
    }

    void Print(std::ostream & os=std::cout, const std::string & empty="-", const std::string & spacer=" ") {
      popM.Print(os, empty, spacer);
    }

    void PrintOrgCounts(const std::function<std::string(ORG*)> & print_fun, std::ostream & os=std::cout) {
      popM.PrintOrgCounts(print_fun, os);
    }

    // Selection mechanisms choose organisms for the next generation.

    // Elite Selection picks a set of the most fit individuals from the population to move to
    // the next generation.  Find top e_count individuals and make copy_count copies of each.
    void EliteSelect(const fit_fun_t & fit_fun, size_t e_count=1, size_t copy_count=1) {
      emp_assert(fit_fun);
      emp_assert(e_count > 0 && e_count <= popM.size());
      // Load the population into a multimap, sorted by fitness.
      std::multimap<double, size_t> fit_map;
      for (size_t i = 0; i < popM.size(); i++) {
        if (this->IsOccupied(i)){
          fit_map.insert( std::make_pair( fitM.CalcFitness(i,popM[i].AsPtr(),fit_fun), i) );
        }
      }

      // Grab the top fitnesses and move them into the next generation.
      auto m = fit_map.rbegin();
      for (size_t i = 0; i < e_count; i++) {
        InsertBirth( *(popM[m->second]), m->second, copy_count);
        ++m;
      }
    }

    // Elite Selection can use the default fitness function.
    void EliteSelect(size_t e_count=1, size_t copy_count=1) {
      EliteSelect(orgM.GetFitFun(), e_count, copy_count);
    }

    // Roulette Selection (aka Fitness-Proportional Selection) chooses organisms to
    // reproduce based on their current fitness.
    void RouletteSelect(const fit_fun_t & fit_fun, size_t count=1) {
      emp_assert(fit_fun);
      emp_assert(count > 0);
      emp_assert(random_ptr != nullptr && "RouletteSelect() requires active random_ptr");

      popM.CalcFitnessAll(fit_fun);

      for (size_t n = 0; n < count; n++) {
        const double fit_pos = random_ptr->GetDouble(fitM.GetTotalFitness());
        size_t id = fitM.At(fit_pos);
        InsertBirth( *(popM[id]), id, 1, fit_fun );
      }
    }

    // Tournament Selection creates a tournament with a random sub-set of organisms,
    // finds the one with the highest fitness, and moves it to the next generation.
    // User provides the fitness function, the tournament size, and (optionally) the
    // number of tournaments to run.
    void TournamentSelect(const fit_fun_t & fit_fun, size_t t_size, size_t tourny_count=1) {
      emp_assert(fit_fun);
      emp_assert(t_size > 0 && t_size <= popM.size(), t_size, popM.size());
      emp_assert(random_ptr != nullptr && "TournamentSelect() requires active random_ptr");

      emp::vector<size_t> entries;
      for (size_t T = 0; T < tourny_count; T++) {
        entries.resize(0);
        for (size_t i=0; i<t_size; i++) entries.push_back( popM.GetRandomOrg() ); // Allows replacement!

        double best_fit = popM.CalcFitness(entries[0], fit_fun);
        size_t best_id = entries[0];

        // Search for a higher fit org in the tournament.
        for (size_t i = 1; i < t_size; i++) {
          const double cur_fit = popM.CalcFitness(entries[i], fit_fun);
          if (cur_fit > best_fit) {
            best_fit = cur_fit;
            best_id = entries[i];
          }
        }

        // Place the highest fitness into the next generation!
        InsertBirth( *(popM[best_id]), best_id, 1 );
      }
    }

    // Tournament Selection can use the default fitness function.
    void TournamentSelect(size_t t_size, size_t tourny_count=1) {
      TournamentSelect(orgM.GetFitFun(), t_size, tourny_count);
    }


    // Run tournament selection with fitnesses adjusted by Goldberg and
    // Richardson's fitness sharing function (1987)
    // Requires a distance function that is valid for members of the population,
    // a sharing threshold (sigma share) that defines which members are
    // in the same niche, and a value of alpha (which controls the shape of
    // the fitness sharing curve
    void FitnessSharingTournamentSelect(const fit_fun_t & fit_fun, const dist_fun_t & dist_fun,
          double sharing_threshhold, double alpha, size_t t_size, size_t tourny_count=1)
    {
      emp_assert(t_size > 0 && t_size <= popM.size());

      // Pre-calculate fitnesses.
      emp::vector<double> fitness(popM.size());
      for (size_t i = 0; i < popM.size(); ++i) {
        double niche_count = 0;
        for (size_t j = 0; j < popM.size(); ++j) {
          double dij = dist_fun(popM[i], popM[j]);
          niche_count += std::max(1 - std::pow(dij/sharing_threshhold, alpha), 0.0);
        }
        fitness[i] = fit_fun(popM[i])/niche_count;
      }

      fitM.Set(fitness);                                // Cache all calculated fitnesses.
      TournamentSelect(fit_fun, t_size, tourny_count);
    }

    // Fitness sharing Tournament Selection can use the default fitness function
    void FitnessSharingTournamentSelect(const dist_fun_t & dist_fun, double sharing_threshold,
          double alpha, size_t t_size, size_t tourny_count=1) {
      FitnessSharingTournamentSelect(orgM.GetFitFun(), dist_fun, sharing_threshold, alpha, t_size, tourny_count);
    }


    // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
    // functions.  The best individuals on each supplemental function divide up a resource pool.
    // NOTE: You must turn off the FitnessCache for this function to work properly.
    void EcoSelect(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
                   const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp_assert(fit_fun);
      emp_assert(t_size > 0 && t_size <= popM.size(), t_size, popM.size());
      emp_assert(random_ptr != nullptr && "EcoSelect() requires active random_ptr");
      emp_assert(fitM.IsCached() == false, "Ecologies mean constantly changing fitness!");

      // Setup info to track fitnesses.
      emp::vector<double> base_fitness(popM.size());
      emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
      emp::vector<double> max_extra_fit(extra_funs.size(), 0.0);
      emp::vector<size_t> max_count(extra_funs.size(), 0);
      for (size_t i=0; i < extra_funs.size(); i++) {
        extra_fitnesses[i].resize(popM.size());
      }

      // Collect all fitness info.
      for (size_t org_id = 0; org_id < popM.size(); org_id++) {
        base_fitness[org_id] = popM.CalcFitness(org_id, fit_fun);
        for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
          double cur_fit = popM.CalcFitness(org_id, extra_funs[ex_id]);
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
        const double cur_bonus = pool_sizes[ex_id] / (double) max_count[ex_id];
        // std::cout << "Bonus " << ex_id << " = " << cur_bonus
        //           << "   max_extra_fit = " << max_extra_fit[ex_id]
        //           << std::endl;

        for (size_t org_id = 0; org_id < popM.size(); org_id++) {
          // If this organism is the best at the current resource, git it the bonus!
          if (extra_fitnesses[ex_id][org_id] == max_extra_fit[ex_id]) {
            base_fitness[org_id] += cur_bonus;
          }
        }
      }


      emp::vector<size_t> entries;
      for (size_t T = 0; T < tourny_count; T++) {
        entries.resize(0);
        for (size_t i=0; i<t_size; i++) entries.push_back( popM.GetRandomOrg() ); // Allows replacement!

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
        InsertBirth( *(popM[best_id]), best_id, 1 );
      }
    }

    /// EcoSelect can be provided a single value if all pool sizes are identical.
    void EcoSelect(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
                   double pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp::vector<double> pools(extra_funs.size(), pool_sizes);
      EcoSelect(fit_fun, extra_funs, pools, t_size, tourny_count);
    }

    // EcoSelect works like Tournament Selection, but also uses a vector of supplimentary fitness
    // functions.  The best individuals on each supplemental function divide up a resource pool.
    // NOTE: You must turn off the FitnessCache for this function to work properly.
    void EcoSelectGradation(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
                   const emp::vector<double> & pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp_assert(fit_fun);
      emp_assert(t_size > 0 && t_size <= popM.size(), t_size, popM.size());
      emp_assert(random_ptr != nullptr && "EcoSelect() requires active random_ptr");
      emp_assert(fitM.IsCached() == false, "Ecologies mean constantly changing fitness!");

      // Setup info to track fitnesses.
      emp::vector<double> base_fitness(popM.size());
      emp::vector< emp::vector<double> > extra_fitnesses(extra_funs.size());
      emp::vector<double> sum_extra_fit(extra_funs.size(), 0.0);
    //   emp::vector<double> resource_left = pool_sizes;
      emp::vector<size_t> max_count(extra_funs.size(), 0);
      for (size_t i=0; i < extra_funs.size(); i++) {
        extra_fitnesses[i].resize(popM.size());
      }
      emp::vector<size_t> ordering(popM.size());


      // Collect all fitness info.
      for (size_t org_id = 0; org_id < popM.size(); org_id++) {
        base_fitness[org_id] = popM.CalcFitness(org_id, fit_fun);
        ordering[org_id] = org_id;
        for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
          double cur_fit = popM.CalcFitness(org_id, extra_funs[ex_id]);
        //   std::cout << cur_fit << " ";
          extra_fitnesses[ex_id][org_id] = emp::Pow(cur_fit, 2.0);
          sum_extra_fit[ex_id] += emp::Pow(cur_fit, 2.0);
        }
      }

      for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {

          for (size_t org_id : ordering) {
              double bonus = (extra_fitnesses[ex_id][org_id] / sum_extra_fit[ex_id]) * pool_sizes[ex_id];
              extra_fitnesses[ex_id][org_id] = emp::Pow(2.0, bonus)-1;
              //   resource_left[ex_id] -= bonus;
              base_fitness[org_id] += bonus;
          }

      }

    //   for (size_t ex_id = 0; ex_id < extra_funs.size(); ex_id++) {
      //
    //     std::cout << "Bonus " << ex_id << " = " << to_string(extra_fitnesses[ex_id])
    //               << std::endl;
    //   }


      emp::vector<size_t> entries;
      for (size_t T = 0; T < tourny_count; T++) {
        entries.resize(0);
        for (size_t i=0; i<t_size; i++) entries.push_back( popM.GetRandomOrg() ); // Allows replacement!

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
        InsertBirth( *(popM[best_id]), best_id, 1 );
      }
    }

    /// EcoSelect can be provided a single value if all pool sizes are identical.
    void EcoSelectGradation(fit_fun_t fit_fun, const emp::vector<fit_fun_t> & extra_funs,
                   double pool_sizes, size_t t_size, size_t tourny_count=1)
    {
      emp::vector<double> pools(extra_funs.size(), pool_sizes);
      EcoSelectGradation(fit_fun, extra_funs, pools, t_size, tourny_count);
    }

    /// LexicaseSelect runs through multiple fitness functions in a random order for
    /// EACH offspring produced.
    // NOTE: You must turn off the FitnessCache for this function to work properly.
    void LexicaseSelect(const emp::vector<fit_fun_t> & fit_funs, size_t repro_count=1)
    {
      emp_assert(popM.size() > 0);
      emp_assert(fit_funs.size() > 0);
      emp_assert(random_ptr != nullptr && "LexicaseSelect() requires active random_ptr");
      emp_assert(fitM.IsCached() == false, "Lexicase constantly changes fitness functions!");

      // Collect all fitness info.
      emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
      for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
        fitnesses[fit_id].resize(popM.size());
        for (size_t org_id = 0; org_id < popM.size(); ++org_id) {
          fitnesses[fit_id][org_id] = popM.CalcFitness(org_id, fit_funs[fit_id]);
        }
      }

      // Go through a new ordering of fitness functions for each selections.
      // @CAO: Can probably optimize a bit!
      // std::set<emp::vector<size_t>> permute_set;
      emp::vector<size_t> all_orgs(popM.size()), cur_orgs, next_orgs;
      for (size_t org_id = 0; org_id < popM.size(); org_id++) all_orgs[org_id] = org_id;

      for (size_t repro = 0; repro < repro_count; ++repro) {
        // Determine the current ordering of the functions.
        emp::vector<size_t> order = GetPermutation(*random_ptr, fit_funs.size());

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
          if (next_orgs.size() == 0) {
              next_orgs.resize(0);
              break;
          }
          // Make next_orgs into new cur_orgs; make cur_orgs allocated space for next_orgs.
          std::swap(cur_orgs, next_orgs);
          next_orgs.resize(0);
        }

        // Place a random survivor (all equal) into the next generation!
        emp_assert(cur_orgs.size() > 0, cur_orgs.size(), fit_funs.size(), all_orgs.size());
        size_t repro_id = cur_orgs[ random_ptr->GetUInt(cur_orgs.size()) ];
        InsertBirth( *(popM[repro_id]), repro_id, 1 );
      }
    }

    /// An Ecological version of Lexicase selection.
    // NOTE: You must turn off the FitnessCache for this function to work properly.
    void EcocaseSelect(const emp::vector<fit_fun_t> & fit_funs,
                       const emp::vector<double> & probs,   // Probability of using each function.
                       double repro_decay,                  // Reduced chance of being used again.
                       size_t repro_count)
    {
      emp_assert(popM.size() > 0);
      emp_assert(fit_funs.size() > 0 && fit_funs.size() == probs.size());
      emp_assert(repro_decay > 0.0 && repro_decay <= 1.0, repro_decay);
      emp_assert(random_ptr != nullptr && "EcocaseSelect() requires active random_ptr");
      emp_assert(fitM.IsCached() == false, "Ecocase constantly changes fitness functions!");

      // Collect all fitness info.
      emp::vector< emp::vector<double> > fitnesses(fit_funs.size());
      for (size_t fit_id = 0; fit_id < fit_funs.size(); ++fit_id) {
        fitnesses[fit_id].resize(popM.size());
        for (size_t org_id = 0; org_id < popM.size(); ++org_id) {
          fitnesses[fit_id][org_id] = popM.CalcFitness(org_id, fit_funs[fit_id]);
        }
      }

      // Determine how eligible each org is for being reproduced.
      emp::vector<double> repro_prob(popM.size(), 1.0);

      // Go through a new ordering of fitness functions for each selection.
      emp::vector<size_t> all_orgs(popM.size()), cur_orgs, next_orgs;
      for (size_t org_id = 0; org_id < popM.size(); org_id++) all_orgs[org_id] = org_id;

      for (size_t repro = 0; repro < repro_count; ++repro) {
        // Determine the current ordering of the functions.
        emp::vector<size_t> order = GetPermutation(*random_ptr, fit_funs.size());

        // Determine the starting set of organisms.
        cur_orgs.resize(0);
        for (size_t org_id = 0; org_id < popM.size(); org_id++) {
          if (repro_prob[org_id] == 1.0 || random_ptr->P(repro_prob[org_id])) {
            cur_orgs.push_back(org_id);
          }
        }

        // Step through the functions in the proper order.
        for (size_t fit_id : order) {
          // Determine if we should skip this fitness function.
          if ( random_ptr->P(1.0 - probs[fit_id]) ) continue;
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
        }

        // Place a random survivor (all equal) into the next generation!
        emp_assert(cur_orgs.size() > 0, cur_orgs.size(), fit_funs.size(), all_orgs.size());
        size_t repro_id = cur_orgs[ random_ptr->GetUInt(cur_orgs.size()) ];
        InsertBirth( *(popM[repro_id]), repro_id, 1 );

        // Reduce the probability of this organism reproducing again.
        repro_prob[repro_id] *= repro_decay;
      }
    }

    // Update() moves the next population to the current position, managing memory as needed.
    void Update() {
    //   std::cout<< "triggering update" << std::endl;
      on_update_sig.Trigger(update);
      update++;
      popM.Update();
    //   std::cout<< "done triggering update" << std::endl;
    }


    // Execute() runs the Execute() method on all organisms in the population, forwarding
    // any arguments.
    template <typename... ARGS>
    void Execute(ARGS &&... args) {
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
