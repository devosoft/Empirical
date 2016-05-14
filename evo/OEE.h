//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

//This file defines an object that will listen to the signals given by a World
//object, calculate metrics of open-ended evolution, and print them out.

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

  template <typename ORG, int MAX_ORG_SIZE, typename... MANAGERS>
  class OEEStatsManager {
  private:
    using skeleton_type = emp::array<int, MAX_ORG_SIZE>;
    using pop_manager_type = AdaptTemplate< typename SelectPopManager<MANAGERS..., PopBasic>::type, ORG>;
    static constexpr bool separate_generations = pop_manager_type::emp_has_separate_generations;
    static constexpr bool emp_is_stats_manager = true;
    //TODO: Make this use existing lineage tracker if there is one
    LineageTracker<ORG> lineage;
    std::set<skeleton_type > novel;

    // The generation we're currently working with - we need this to
    // track lineage
    emp::vector<int> generation_since_update;

    // Historical generations needed to count stats. We only need these in
    // proportion to resolution.
    int generations = 50; //How far back do we look for persistance?
    int resolution = 10; //With what frequency do we record data?
    std::deque<emp::vector<int> > past_snapshots;

    //int update = 0;
    int next_parent_id = -1;
    int next_org_id;

  public:

    OEEStatsManager(World<ORG, MANAGERS...>* world){
      // This isn't going to work if generations aren't a multiple of resolution
      emp_assert(generations % resolution == 0 &&
          "ERROR: Generations required for persistance must be a multiple of resolution.");

      past_snapshots = std::deque<emp::vector<int> >(2*generations/resolution + 1);

      //Create std::function objects for all the callbacks. It seems like
      //this maybe shouldn't be necessary (or at least shouldn't need to happen
      //in the constructor), but for now it is or the compiler throws
      //internal errors
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

      //Setup signal callbacks
      world->OnBeforeRepro(RecordParentFun);
      world->OnOffspringReady(TrackOffspringFun);
      world->OnInjectReady(TrackInjectedOffspringFun);
      world->OnUpdate(UpdateFun);
      world->OnOrgPlacement(TrackPlacementFun);

      //TODO: Figure out how to make this work automatically
      //fit_fun = world->GetFitFun();
    }

    //Put newly born organism into the lineage tracker
    std::function<double(ORG * org)> fit_fun;
    void TrackOffspring(ORG * org) {
      next_org_id = lineage.AddOrganism(*org, next_parent_id);

    }

    //Put newly injected organism into the lineage tracker
    void TrackInjectedOffspring(ORG * org) {
      //std::cout << "Track Injected" << std::endl;
      next_org_id = lineage.AddOrganism(*org, -1);

    }

    //Keep track of location of all orgs in the population so that
    //we can translate their ids from the World to ids within the lineage
    //tracker
    void TrackPlacement(int pos) {
      if (pos >= generation_since_update.size()) {
        generation_since_update.resize(pos+1);
      }
      generation_since_update[pos] = next_org_id;
    }

    //Record the org that's about to have an offspring, so we can know
    //who the parent of the next org is.
    void RecordParent(int id) {
      if (separate_generations){
        next_parent_id = past_snapshots.back()[id];
      } else {
        next_parent_id = generation_since_update[id];
      }
    }

    //Update callback function handles calculating stats
    void Update(int update) {
      int change = -1;
      int novelty = -1;
      double ecology = -1;
      int complexity = -1;
      double fittest = -1;
      if (update % resolution == 0) {

        emp::vector<skeleton_type > persist_skeletons = Skeletonize(
                                              GetPersistLineage(&lineage,
                                              past_snapshots[0],
                                              past_snapshots[generations/resolution]));
        emp::vector<skeleton_type > prev_persist_skeletons = Skeletonize(
                                              GetPersistLineage(&lineage,
                                              past_snapshots[generations/resolution],
                                              past_snapshots[2*generations/resolution]));

        if (past_snapshots[2*generations/resolution].size() > 0){
          change = ChangeMetric(persist_skeletons, prev_persist_skeletons);
        }

        if (past_snapshots[generations/resolution].size() > 0){
          novelty = NoveltyMetric(persist_skeletons);
          ecology = EcologyMetric(persist_skeletons);
          complexity = ComplexityMetric(persist_skeletons,
                              [](skeleton_type skel){
                                int nulls = std::count(skel.begin(), skel.end(), -1);
                                return (double)(skel.size() - nulls);
                              });
          fittest = MaxFitness(fit_fun, IDsToGenomes(&lineage, past_snapshots[0]));
        }
        std::cout << "Update: " << update << ", Change: " << change << ", Novelty: " << novelty << ", Ecology: " << ecology << ", Complexity: " << complexity << ", MaxFitness: " << fittest << std::endl;
        past_snapshots.pop_back();
        past_snapshots.push_front(generation_since_update);
      }

      if (separate_generations) {
        //TODO: This isn't sufficient - need to add signals for any
        //population change event
        generation_since_update.resize(0);
      }
    }


    //Convert a container of orgs to skeletons containing only informative sites
    //TODO: Currently assumes bit org
    template <template <typename> class C >
    C<skeleton_type> Skeletonize (C<ORG> orgs){
      C<skeleton_type> skeletons;
      for (auto org : orgs) {
        double fitness = fit_fun(&org);
        skeleton_type skeleton;
        ORG test = org;

        for (int i = 0; i < org.size(); i++) {
          test[i] = !test[i];
          if (fit_fun(&test) >= fitness){
            skeleton[i] = -1;
          } else {
            skeleton[i] = org[i];
          }
          test[i] = !test[i];
        }
        skeletons.push_back(skeleton);
      }
      return skeletons;
    }

    //Find most complex skeleton in the given vector.
    double ComplexityMetric(emp::vector<skeleton_type> persist,
                            std::function<double(skeleton_type)> complexity_fun) {

      double most_complex = complexity_fun(*(persist.begin()));

      for (auto org : persist) {
        if (complexity_fun(org) > most_complex) {
          most_complex = complexity_fun(org);
        }
      }
      return most_complex;
    }

    //Determine the shannon diversity of the skeletons in the given vector
    double EcologyMetric(emp::vector<skeleton_type> persist){

      return emp::evo::ShannonDiversity(persist);

    }

    //Determine how many skeletons the given vector contains that have never
    //been seen before
    int NoveltyMetric(emp::vector<skeleton_type > persist){

      int result = 0;

      for (skeleton_type lin : persist){
        if (novel.find(lin) == novel.end()){
          result++;
          novel.insert(lin);
        }
      }

      return result;
    }

    //How many skeletons are there in the first vector that aren't in the second?
    int ChangeMetric( emp::vector<skeleton_type > persist,
                      emp::vector<skeleton_type > prev_persist){

      std::set<skeleton_type > curr_set(persist.begin(), persist.end());
      std::set<skeleton_type > prev_set(prev_persist.begin(), prev_persist.end());

      std::set<skeleton_type > result;
      std::set_difference(persist.begin(), persist.end(), prev_persist.begin(),
      prev_persist.end(), std::inserter(result, result.end()));
      return result.size();
    }

  };

  //Here lies the beastiary of functions for dealing with persistant lineages

  //Takes a container of ints representing org ids (as assigned by the lineage)
  //tracker, and returns a contatiner of the genomes of those ints.
  template <typename GENOME, template <typename> class C >
  C<GENOME> IDsToGenomes(LineageTracker<GENOME>* lineages, C<int> persist_ids) {
    C<GENOME> persist;
    for (int id : persist_ids){
      persist.insert(*(lineages->org_to_genome[id]));
    }

    return persist;
  }

  //Specialization for emp::vector so we can use push_back
  template <typename GENOME>
  emp::vector<GENOME> IDsToGenomes(LineageTracker<GENOME>* lineages, emp::vector<int> persist_ids) {
    emp::vector<GENOME> persist;
    for (int id : persist_ids){
      persist.push_back(*(lineages->org_to_genome[id]));
    }

    return persist;
  }

  //GET PERSISTANT LINEAGE IDS FUNCTIONS:
  //These functions get the ids of orgs that went on to be the ancestor of a
  //lineage that persisted for the indicated amount of time. They will accept
  //any container of ints representing org ids and return the same container

  //The first version takes the current generation and an int indicating
  //how many generations a lineage must have survived to count as persistent.
  //It returns the ids of all orgs that were exactly that many generations
  //back in a lineage

  //Generic container version
  template <typename GENOME, template <typename> class C >
  C<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
               C<int> curr_generation,
               int generations){

    C<int> persist;
    for (int id : curr_generation){
      emp::vector<int> lin = lineages->TraceLineageIDs(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(*(lin.begin() + generations));
    }

    return persist;
  }

  //emp::vector version so we can use push_back
  template <typename GENOME>
  emp::vector<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
               emp::vector<int> curr_generation,
               int generations){

    emp::vector<int> persist;
    for (int id : curr_generation){
      emp::vector<int> lin = lineages->TraceLineageIDs(id);
      emp_assert(lin.size() - generations > 0);
      persist.push_back(*(lin.begin() + generations));
    }

    return persist;
  }

  //The second version takes two containers of ints representing the ids of
  //the orgnaisms that were present at one point in time and a point in time
  //the desired length of time ago, and determines which orgs in the second
  //container have descendants in the first.

  template <typename GENOME, template <typename> class C >
  C<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
                C<int> curr_generation,
                C<int> prev_generation){

    C<int> persist;

    for (auto id : curr_generation){
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

  //Specialization for emp::vector so we can use push_back
  template <typename GENOME>
  emp::vector<int> GetPersistLineageIDs(LineageTracker<GENOME>* lineages,
                emp::vector<int> curr_generation,
                emp::vector<int> prev_generation){

    emp::vector<int> persist;

    for (auto id : curr_generation){
      while(id) {

        if (std::find(prev_generation.begin(), prev_generation.end(), id) != prev_generation.end()) {
          persist.push_back(id);
          break;
        }
        id = lineages->parents[id];
      }
    }

    return persist;
  }

  //Whereas GetPersistLineageIDs returns the ids of the orgs that are the
  //ancestors of persistent lineages, GetPersistLineage converts the ids to
  //GENOMEs first.
  template <typename GENOME, template <typename> class C >
  C<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages,
				       C<int> curr_generation,
				       int generations){

    C<GENOME> persist;
    for (int id : curr_generation){
      emp::vector<GENOME*> lin = lineages->TraceLineage(id);
      emp_assert(lin.size() - generations > 0);
      persist.insert(**(lin.begin() + generations));
    }

    return persist;
  }

  //Version that takes two populations
  template <typename GENOME, template <typename> class C >
  C<GENOME> GetPersistLineage(LineageTracker<GENOME>* lineages,
                C<int> curr_generation,
                C<int> prev_generation){

    C<int> persist_ids = GetPersistLineageIDs(lineages, curr_generation, prev_generation);
    return IDsToGenomes(lineages, persist_ids);
  }


}
}


#endif
