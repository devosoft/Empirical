//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#ifndef EMP_LINEAGE_TRACKER_H
#define EMP_LINEAGE_TRACKER_H

#include <map>
#include <set>
#include "../tools/vector.h"
#include "PopulationManager.h"

namespace emp{
namespace evo{

  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class LineageTracker_Null {
  public:
    static constexpr bool emp_is_lineage_manager = true;
    LineageTracker_Null(){;};

    template <typename WORLD>
    void Setup(WORLD * w){;}
  };

  // Class to keep track of lineages
  // Maintains record of all genomes that ever existed, which organisms
  // they belonged to, and which organisms were the parents of which
  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class LineageTracker {
  protected:
    using org_ptr = typename POP_MANAGER::value_type;
    using ORG = typename std::remove_pointer<org_ptr>::type;
    static constexpr bool separate_generations = POP_MANAGER::emp_has_separate_generations;
  public:
    static constexpr bool emp_is_lineage_manager = true;
    std::set<ORG> genomes;
    std::map<int, org_ptr> org_to_genome;
    std::map<int, int> parents;
    int next = 1; //0 indicates no parent
    int next_parent_id = -1;
    int next_org_id = 1;
    emp::vector<int> generation_since_update;
    emp::vector<int> new_generation;
    bool inject;

    LineageTracker(){;}

    template <typename WORLD>
    LineageTracker(WORLD * w) {

      //Create std::function objects for all the callbacks. It seems like
      //this maybe shouldn't be necessary (or at least shouldn't need to happen
      //in the constructor), but for now it is or the compiler throws
      //internal errors
      Setup(w);
    }

    template <typename WORLD>
    void Setup(WORLD * w){
      std::function<void(int)> RecordParentFun = [this] (int id){
        RecordParent(id);
      };

      std::function<void(int)> TrackPlacementFun = [this] (int pos){
        TrackPlacement(pos);
      };

      std::function<void(org_ptr)> TrackOffspringFun = [this] (org_ptr org){
        TrackOffspring(org);
      };

      std::function<void(org_ptr)> TrackInjectedOffspringFun = [this] (org_ptr org){
        TrackInjectedOffspring(org);
      };

      std::function<void(int)> UpdateFun = [this] (int ud){
        Update(ud);
      };

      w->OnBeforeRepro(RecordParentFun);
      w->OnOffspringReady(TrackOffspringFun);
      w->OnInjectReady(TrackInjectedOffspringFun);
      w->OnOrgPlacement(TrackPlacementFun);
      w->OnUpdate(UpdateFun);
    }

    ~LineageTracker() {
      //for (GENOME g : genomes) delete &g;
    }

    //Put newly born organism into the lineage tracker

    void Update(int i) {
      if (separate_generations) {
        //TODO: This isn't sufficient - need to add signals for any
        //population change event
        generation_since_update = new_generation;
        new_generation.resize(0);
      }
    }

    void TrackOffspring(org_ptr org) {
      next_org_id = this->AddOrganism(*org, next_parent_id);
      inject = false;
    }

    //Put newly injected organism into the lineage tracker
    void TrackInjectedOffspring(org_ptr org) {
      next_org_id = this->AddOrganism(*org, 0);
      inject = true;
    }

    //Keep track of location of all orgs in the population so that
    //we can translate their ids from the World to ids within the lineage
    //tracker
    void TrackPlacement(int pos) {
      if (separate_generations && !inject){
        if (pos >= new_generation.size()) {
          new_generation.resize(pos+1);
        }
        new_generation[pos] = next_org_id;

      } else {
        if (pos >= generation_since_update.size()) {
          generation_since_update.resize(pos+1);
        }
        generation_since_update[pos] = next_org_id;
      }
    }

    //Record the org that's about to have an offspring, so we can know
    //who the parent of the next org is.
    void RecordParent(int id) {
      next_parent_id = generation_since_update[id];
    }

    // Add an organism to the tracker - org is the genome of the organism
    // and parent is the id of the parent. The lineage tracker is in charge
    // of assigning ids, and will return an int representing the id of the
    // organism you added
    int AddOrganism(ORG org, int parent) {
      int id = this->next++;
      std::pair<typename std::set<ORG>::iterator, bool> ret;
      ret = genomes.insert(org);
      typename std::set<ORG>::iterator it = ret.first;
      org_ptr genome = (org_ptr)&(*it);
      this->org_to_genome[id] = genome;
      this->parents[id] = parent;
      return id;
    }

    // Return a vector containing the genomes of an organism's ancestors
    emp::vector<org_ptr> TraceLineage(int org_id) {
      emp::vector<org_ptr> lineage;

      while(org_id) {
        lineage.push_back(this->org_to_genome[org_id]);
        org_id = this->parents[org_id];
      }

      return lineage;

    }

    //Return a vector containing the IDs of an oraganism's ancestors
    emp::vector<int> TraceLineageIDs(int org_id) {
      emp::vector<org_ptr> lineage;

      while(org_id) {
        lineage.push_back(org_id);
        org_id = this->parents[org_id];
      }

      return lineage;

    }

  };


  template <typename POP_MANAGER = PopulationManager_Base<int> >
  class LineageTracker_Pruned : LineageTracker<POP_MANAGER> {
  protected:
    std::set<int> alive;
    std::map<int, int> offspring;
    std::map<int, int> oldest_unique;
    using org_ptr = typename LineageTracker<POP_MANAGER>::org_ptr;
    using ORG = typename LineageTracker<POP_MANAGER>::ORG;

    using LineageTracker<POP_MANAGER>::next_org_id;
    using LineageTracker<POP_MANAGER>::next_parent_id;
    using LineageTracker<POP_MANAGER>::generation_since_update;
    using LineageTracker<POP_MANAGER>::separate_generations;
    using LineageTracker<POP_MANAGER>::genomes;
    using LineageTracker<POP_MANAGER>::parents;
    using LineageTracker<POP_MANAGER>::org_to_genome;
    using LineageTracker<POP_MANAGER>::new_generation;

    std::map<ORG, int> genome_counts;


  public:

    using LineageTracker<POP_MANAGER>::emp_is_lineage_manager;
    LineageTracker_Pruned() {;}

    template <typename WORLD>
    LineageTracker_Pruned(WORLD * w) {

      //Create std::function objects for all the callbacks. It seems like
      //this maybe shouldn't be necessary (or at least shouldn't need to happen
      //in the constructor), but for now it is or the compiler throws
      //internal errors
      Setup(w);
    }

    template <typename WORLD>
    void Setup(WORLD * w){
      std::cout << "Setting up" << std::endl;
      std::function<void(int)> RecordParentFun = [this] (int id){
        RecordParent(id);
      };

      std::function<void(int)> TrackPlacementFun = [this] (int pos){
        TrackPlacement(pos);
      };

      std::function<void(org_ptr)> TrackOffspringFun = [this] (org_ptr org){
        TrackOffspring(org);
      };

      std::function<void(org_ptr)> TrackInjectedOffspringFun = [this] (org_ptr org){
        TrackInjectedOffspring(org);
      };

      std::function<void(int)> UpdateFun = [this] (int ud){
        Update(ud);
      };

      w->OnBeforeRepro(RecordParentFun);
      w->OnOffspringReady(TrackOffspringFun);
      w->OnInjectReady(TrackInjectedOffspringFun);
      w->OnOrgPlacement(TrackPlacementFun);
      w->OnUpdate(UpdateFun);
    }

    ~LineageTracker_Pruned() {
      //for (GENOME g : genomes) delete &g;
    }


    void TrackOffspring(org_ptr org) {
      next_org_id = this->AddOrganism(*org, next_parent_id);
    }

    //Put newly injected organism into the lineage tracker
    void TrackInjectedOffspring(org_ptr org) {
      next_org_id = this->AddOrganism(*org, 0);
    }

    //Keep track of location of all orgs in the population so that
    //we can translate their ids from the World to ids within the lineage
    //tracker
    void TrackPlacement(int pos) {

      //Once things can die we'll need something better here
      std::cout << "Placing" << std::endl;
      if (generation_since_update.size() > pos){
        std::cout << pos << std::endl;
        int id = generation_since_update[pos];
        alive.erase(id);
        std::cout << id << " is being killed" << std::endl;
        while (!offspring[id] && !alive.count(id)) {
          std::cout << "Evaluating: " << id << std::endl;
          offspring[parents[id]]--;
          std::cout << offspring[parents[id]] << " siblings left" << std::endl;
          parents.erase(id);
          org_to_genome.erase(id);
          ORG genome = *(org_to_genome[id]);
          genome_counts[genome]--;
          std::cout << genome_counts[genome] << " remaining identical genotypes" << std::endl;
          if (!genome_counts[genome]) {
            genomes.erase(genome);
          }
          id = parents[id];
          std::cout << "Parent: " << id << std::endl;
        }
      }

      if (pos >= generation_since_update.size()) {
        generation_since_update.resize(pos+1);
      }
      generation_since_update[pos] = next_org_id;
    }

    //Record the org that's about to have an offspring, so we can know
    //who the parent of the next org is.
    void RecordParent(int id) {
      offspring[id]++;
      next_parent_id = generation_since_update[id];
    }

    // Add an organism to the tracker - org is the genome of the organism
    // and parent is the id of the parent. The lineage tracker is in charge
    // of assigning ids, and will return an int representing the id of the
    // organism you added
    int AddOrganism(ORG org, int parent) {
      int id = this->next++;
      std::pair<typename std::set<ORG>::iterator, bool> ret;
      ret = genomes.insert(org);
      typename std::set<ORG>::iterator it = ret.first;
      org_ptr genome = (org_ptr)&(*it);
      this->org_to_genome[id] = genome;

      if (ret.second) {
        genome_counts[*genome] = 0;
      } else {
        genome_counts[*genome]++;
      }

      this->parents[id] = parent;
      alive.insert(id);
      offspring[id] = 0;
      return id;
    }


    void Update(int i) {
      if (separate_generations) {
        //TODO: This isn't sufficient - need to add signals for any
        //population change event
        generation_since_update = new_generation;
        new_generation.resize(0);
      }
    }

};


  using LineageNull = LineageTracker_Null<PopBasic>;
  using LineageStandard = LineageTracker<PopBasic>;
  using LineagePruned = LineageTracker_Pruned<PopBasic>;
}
}

#endif
