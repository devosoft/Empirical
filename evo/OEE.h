//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
#ifndef EMP_OEE_H
#define EMP_OEE_H

#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <iterator>
#include <functional>

#include "../tools/vector.h"
#include "../tools/array.h"
#include "LineageTracker.h"
#include "World.h"
#include "PopulationManager.h"
#include "Stats.h"

namespace emp{
namespace evo{

  //EMP_SETUP_TYPE_SELECTOR(DetermineSeparateGens, emp_has_separate_generations);
  EMP_SETUP_TYPE_SELECTOR(SelectPopManagerOEE, emp_is_population_manager);
  EMP_SETUP_TYPE_SELECTOR(SelectOrgManagerOEE, emp_is_organism_manager);
  //This is all going to get superceded by Cliff's stuff
  //The pop_manager totally knows what the org type is and I'm sure there's
  //a way to get it to tell us rather than making it a separate template
  //argument

  //template <typename ORG, typename... MANAGERS>
  //class World;

  template <typename ORG, typename... MANAGERS>
  class OEEStatsManager {
  private:
    static constexpr bool separate_generations = PopulationManager_Base<int>::emp_has_separate_generations;//SelectPopManagerOEE<MANAGERS..., PopulationManager_Base<int> >::emp_has_separate_generations;
    static constexpr bool emp_is_stats_manager = true;
    //TODO: Make this use existing lineage tracker if there is one
    LineageTracker<ORG> lineage;
    static constexpr int ORGSIZE = 5;
    std::set<emp::array<int, ORGSIZE> > novel;

    //This may or may not get untennably huge
    std::map<ORG, emp::array<int, ORGSIZE> > skeleton_cache;

    // The generation we're currently working with - we need this to
    // track lineage
    emp::vector<int> generation_since_update;

    // Historical generations needed to count stats. We only need these in
    // proportion to resolution.
    int generations = 10; //How far back do we look for persistance?
    int resolution = 10; //With what frequency do we record data?
    std::deque<emp::vector<int> > past_snapshots;

    //int update = 0;
    int next_parent_id = -1;
    int next_org_id;

  public:
    std::function<double(ORG * org)> fit_fun;
    void TrackOffspring(ORG * org) {
      //std::cout << "Track Offspring" << std::endl;
      next_org_id = lineage.AddOrganism(*org, next_parent_id);

    }

    void TrackPlacement(int pos) {
      //std::cout << "Track Placement" << std::endl;
      if (pos >= generation_since_update.size()) {
        generation_since_update.resize(pos+1);
      }
      generation_since_update[pos] = next_org_id;
    }

    void TrackInjectedOffspring(ORG * org) {
      //std::cout << "Track Injected" << std::endl;
      next_org_id = lineage.AddOrganism(*org, -1);

    }

    void Update(int update) {
      std::cout << "Update: " << update << std::endl;
      if (update % resolution == 0) {
        //emp::vector<ORG> curr_gen = Skeletonize(fit_fun, past_snapshots[0]);
        //emp::vector<ORG> prev_gen = Skeletonize(fit_fun, past_snapshots[generations/resolution]);

        std::cout << "Printing stats" << std::endl;
        int change = ChangeMetric(&lineage, past_snapshots[0], past_snapshots[generations/resolution], past_snapshots[2*generations/resolution]);
        std::cout << "Change done" << std::endl;
        int novelty = NoveltyMetric(&lineage, past_snapshots[0], past_snapshots[generations/resolution], &(this->novel));
        std::cout << "Novelty done" << std::endl;
        double ecology = EcologyMetric(&lineage, past_snapshots[0], past_snapshots[generations/resolution]);
        std::cout << "Ecology done" << std::endl;
        double complexity = ComplexityMetric(&lineage, past_snapshots[0], past_snapshots[generations/resolution]);
        std::cout << "Complexity done" << std::endl;
        std::cout << "Update: " << update << " Change: " << change << " Novelty: " << novelty << " Ecology: " << ecology << " Complexity: " << complexity<< std::endl;
        past_snapshots.pop_back();
        past_snapshots.push_front(generation_since_update);
      }

      //update++;

      if (separate_generations) {
        std::cout << "separate generations true" << std::endl;
        //This isn't sufficient - need to add signals for any
        //population change event
        generation_since_update.resize(0);
      }
    }

    void RecordParent(int id) {
      //std::cout << "Record Parent" << std::endl;
      if (separate_generations){
        next_parent_id = past_snapshots.back()[id];
      } else {
        next_parent_id = generation_since_update[id];
      }
    }

    //TODO: Currently assumes bit org
    template <typename C>
    emp::vector<emp::array<int, ORGSIZE> > Skeletonize (std::function<double(ORG*)> fit_fun, C orgs){
      emp::vector<emp::array<int, ORGSIZE> > skeletons;
      for (auto org : orgs) {
        double fitness = fit_fun(&org);
        //TODO: Make this work for non-ints
        emp::array<int, ORGSIZE> skeleton;
        ORG test = org;

        for (int i = 0; i < org.size(); i++) {
          test[i] = !test[i];
          if (fit_fun(&test) >= fitness){
            //std::cout << "Not informative" << std::endl;
            skeleton[i] = -1;
          } else {
            //std::cout << "Informative"<< org[i] << std::endl;
            skeleton[i] = org[i];
          }
          test[i] = !test[i];
        }

        std::cout << "Skeleton: ";
        for (int i : skeleton){
          std::cout << i;
        }
        std::cout << std::endl;
        skeletons.push_back(skeleton);

      }


      return skeletons;
    }

    double ComplexityMetric(LineageTracker<ORG>* lineages,
                            emp::vector<int> curr_generation,
                            emp::vector<int> prev_generation){

      if (prev_generation.size() == 0) {
        return -1;
      }

      std::set<int> curr_set(curr_generation.begin(), curr_generation.end());
      std::set<int> prev_set(prev_generation.begin(), prev_generation.end());

      //Find persistant lineages
      std::set<int> persist = GetPersistLineageIDs(lineages, curr_generation, prev_generation);

      double most_complex = lineages->org_to_genome[prev_generation[0]]->GetSize();

      for (int org : prev_generation) {
        if (std::find(persist.begin(), persist.end(), org) != persist.end()) {
          if (lineages->org_to_genome[org]->GetSize() > most_complex) {
            most_complex = lineages->org_to_genome[org]->GetSize();
          }
        }
      }
      return most_complex;
    }

    double EcologyMetric(LineageTracker<ORG>* lineages,
                          emp::vector<int> curr_generation,
                          emp::vector<int> prev_generation){

      if (prev_generation.size() == 0) {
        return -1;
      }

      //std::set<int> curr_set(curr_generation.begin(), curr_generation.end());
      std::set<int> persist = GetPersistLineageIDs(lineages, curr_generation, prev_generation);
      emp::vector<ORG> culled_generation;

      for (int org : prev_generation) {
        if (std::find(persist.begin(), persist.end(), org) != persist.end()) {
          culled_generation.push_back(*(lineages->org_to_genome[org]));
        }
      }

      return emp::evo::ShannonDiversity(culled_generation);

    }

    int NoveltyMetric(LineageTracker<ORG>* lineages,
                      emp::vector<int> curr_generation,
                      emp::vector<int> prev_generation,
                      std::set<emp::array<int, ORGSIZE> >* novel){

      if (prev_generation.size() == 0) {
        return -1;
      }

      std::set<int> curr_set(curr_generation.begin(), curr_generation.end());
      std::set<int> prev_set(prev_generation.begin(), prev_generation.end());

      std::set<ORG> persist = GetPersistLineage(lineages, curr_set,  prev_set);
      emp::vector<emp::array<int, ORGSIZE> > persist_skeletons = Skeletonize(fit_fun, persist);
      int result = 0;

      for (emp::array<int, ORGSIZE> lin : persist_skeletons){
        if (novel->find(lin) == novel->end()){
          result++;
          novel->insert(lin);
        }
      }

      return result;
    }

    int ChangeMetric(LineageTracker<ORG>* lineages,
                      emp::vector<int> curr_generation,
                      emp::vector<int> prev_generation,
                      emp::vector<int> first_generation){

      if (prev_generation.size() == 0 || first_generation.size() == 0) {
        return -1;
      }

      std::set<int> curr_set(curr_generation.begin(), curr_generation.end());
      std::set<int> prev_set(prev_generation.begin(), prev_generation.end());
      std::set<int> first_set(first_generation.begin(), first_generation.end());

      //Find persistant lineages
      std::set<ORG> persist = GetPersistLineage(lineages, curr_set,  prev_set);
      std::set<ORG> prev_persist = GetPersistLineage(lineages, prev_set, first_set);

      std::set<ORG> result;
      std::set_difference(persist.begin(), persist.end(), prev_persist.begin(),
      prev_persist.end(), std::inserter(result, result.end()));
      return result.size();
    }


    //std::function<void(ORG *)> TrackOffspringFun = TrackOffspring;
    //std::function<void(ORG *)> TrackInjectedOffspringFun = TrackInjectedOffspring;
    //std::function<void(int)> UpdateFun = Update;

    OEEStatsManager(World<ORG, MANAGERS...>* world){
      // This isn't going to work if generations aren't a multiple of resolution
      emp_assert(generations % resolution == 0 && "TEST MESSAGE");

      //for (int i = 0; i < 2*generations/resolution; i++)  {
      //  this->past_snapshots.push_back(emp::vector<int>());
      //}
      past_snapshots = std::deque<emp::vector<int> >(2*generations/resolution);
      std::function<void(int)> RecordParentFun = [this] (int id){
        RecordParent(id);
      };

      std::function<void(int)> TrackPlacementFun = [this] (int pos){
        TrackPlacement(pos);
      };

      std::function<void(ORG *)> TrackOffspringFun = [this] (ORG * org){
        TrackOffspring(org);
      };

      std::function<void(ORG *)> TrackInjectedOffspringFun = [this] (ORG * org){
        TrackInjectedOffspring(org);
      };

      std::function<void(int)> UpdateFun = [this] (int ud){
        Update(ud);
      };

      std::cout << "Doing Setup" << std::endl;
      world->OnBeforeRepro(RecordParentFun);
      world->OnOffspringReady(TrackOffspringFun);
      world->OnInjectReady(TrackInjectedOffspringFun);
      world->OnUpdate(UpdateFun);
      world->OnOrgPlacement(TrackPlacementFun);

      //fit_fun = world->GetFitFun();
    }


  };



  //Returns a set of org ids (from lineage tracker) representing ancestors
  //of the organisms with ids in curr_generation that lived the specified number
  //of generations earlier
  //
  //TODO: This depends on generations right now, which means it won't work
  //with steady-state populations.
  template <typename GENOME>
  std::set<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages,
				       std::set<int> curr_generation,
				       int generations){

    std::set<GENOME> persist;
    for (int id : curr_generation){
      emp::vector<GENOME*> lin = lineages->TraceLineage(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(**(lin.begin() + generations));
    }

    return persist;
  }

  template <typename GENOME, typename C, typename = std::enable_if<std::is_integral<typename C::value_type>::value > >
  std::set<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages,
                C curr_generation,
                C prev_generation){

    std::set<int> persist_ids = GetPersistLineageIDs(lineages, curr_generation, prev_generation);
    return IDsToGenomes(lineages, persist_ids);
  }

  template <typename GENOME, typename C, typename = std::enable_if<std::is_integral<typename C::value_type>::value > >
  std::set<GENOME> IDsToGenomes(LineageTracker<GENOME>*, C persist_ids) {
    std::set<GENOME> persist;
    for (int id : persist_ids){
      persist.insert(*(lineages->org_to_genome[id]));
    }

    return persist;
  }

  template <typename GENOME>
  std::set<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
               std::set<int> curr_generation,
               int generations){

    std::set<int> persist;
    for (int id : curr_generation){
      emp::vector<int> lin = lineages->TraceLineageIDs(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(*(lin.begin() + generations));
    }

    return persist;
  }

  //Can take any container of ints
  template <typename GENOME, typename C, typename = std::enable_if<std::is_integral<typename C::value_type>::value > >
  std::set<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
                C curr_generation,
                C prev_generation){

    std::set<int> persist;

    for (int id : curr_generation){
      while(id) {

        if (std::find(prev_generation.begin(), prev_generation.end(), id) != prev_generation.end()) {
          persist.insert(id);
          break;
        }
        id = lineages->parents[id];
      }
    }

    return persist;
  }


}
}


#endif
