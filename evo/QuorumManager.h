//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains the  manager for quroum organisms

#ifndef EMP_EVO_QUORUM_MANAGER
#define EMP_EVO_QUORUM_MANAGER

#include "../tools/Random.h"
#include "../config/config.h"
#include "QuorumOrg.h"
#include "World.h"
#include <iostream>

/* TODO: determine if MUTATION_RATE etc. should be config'd here or QuorumOrg.h
 *
 * This class will handle all the quorum-specific handling of QOrgs (public goods creation,
 * sensing/informing orgs about quorums) and will template an underlying structural
 * population manager to handle the physical location / neighbors of the orgs.
 */

//using MixedWorld = emp::evo::World<ORG, emp::evo::PopulationManager_Base<ORG>>;
//MixedWorld<BitOrg> mixed_pop(random);


namespace emp {
namespace evo {

  // templated QuorumManager class
  template <class QuorumOrganism, template<class> class POP_MANAGER>
  class QuorumManager : public POP_MANAGER<QuorumOrganism> {
    using POP_MANAGER<QuorumOrganism>::random_ptr;
    // we require that the base class has a get_neighbors methods

  protected:
    using POP_MANAGER<QuorumOrganism>::pop;
    int spacing = -1;
    long available_private_points = 16000;

    /// calculates quorum and updates state.hi_density  
    bool calculate_quorum (std::set<QuorumOrganism *> & neighbors) {
      int active_neighbors = 0;
      int num_hi = 0;
      int potential = POP_MANAGER<QuorumOrganism>::GetMaxPossibleInRadius(nullptr, ai_radius);

      for (auto org_iter : neighbors) {
        if (org_iter == nullptr) {continue;} // ignore nonextant orgs
        if (org_iter->hi_density()) {num_hi++;}
        if (org_iter->lo_density()) {active_neighbors++;}
      }

      // consider quoorum to be enough hi or enough lo in potential radius
      return (double) active_neighbors / (double) potential > lo_weight || 
             (double) num_hi / (double) potential < hi_weight;
    }

    double calculate_quorum (QuorumOrganism * org) {
      bool result;    
      // for an example of an implemented get_org_neighbors see Grid pop manager.
      // PopulationManager.h:209ish
      auto neighbors = POP_MANAGER<QuorumOrganism>::get_org_neighbors(org->get_loc());

      org->set_density(result = calculate_quorum(neighbors)); 
      return result;
    }

public:

    static int hi_weight, lo_weight, ai_radius;
    
    QuorumManager () {
      POP_MANAGER<QuorumOrganism>();
      }

    void SetRandom(Random * rand) {
      random_ptr = rand;
      QuorumOrganism::random = rand;
    }

    void SpacedSeed(double grid_density, int num_locs, QuorumOrgGenome const * seed, bool mut,
                    double alt_density = 0, QuorumOrgGenome const * alt_seed = nullptr) {
      // sanity check
      if(alt_density > 0) { assert(alt_seed != nullptr && "Must have non-null alt-seed when alt-density specified!");}
     
      int num_placed = 0, position = 0, num_alt_placed = 0;
      // there are (num_locs * grid_density) # of slots that'll be occupied w/ orgs
      // they are `spacing` distance apart
      int num_to_place = num_locs * grid_density;
      int spacing = num_locs / num_to_place;
      
      
      // there are (num_locs * grid_density * alt_density) # spaces that'll be occupied
      // w/ alt orgs
      int num_alt = num_to_place * alt_density;

      QuorumOrganism * org;

      while(position < pop.size()) {
        if(alt_density > 0 && (double) num_alt_placed / (double) num_to_place < alt_density) {
          org = new QuorumOrganism(alt_seed->co_op_prob, alt_seed->ai_radius, 
                                   alt_seed->quorum_threshold, mut, 0, alt_seed->lineage, 
                                   alt_seed->can_make_HiAI, alt_seed->can_make_LoAI);
          num_alt_placed++;
        } else {
          org = new QuorumOrganism(seed->co_op_prob, seed->ai_radius,seed->quorum_threshold, mut, 
                                   0, seed->lineage, seed->can_make_HiAI,
                                   seed->can_make_LoAI);
        }

        pop[position] = org;
        pop[position]->set_id(position);
        position += spacing;
        num_placed++;
      }

    }

    void set_available_points(long pts) {available_private_points = pts;}
    long get_available_points () const {return available_private_points;}


    int classify (QuorumOrganism const * org) {
      return QuorumOrganism::classify(org);
    }

    // minor override to the parent class to save the orgs location to the org
    unsigned int AddOrg (QuorumOrganism * org) {
      return org->set_id(POP_MANAGER<QuorumOrganism>::AddOrg(org));
    }

    unsigned int AddOrgBirth (QuorumOrganism * offspring, int parent_pos) {
      return offspring->set_id(POP_MANAGER<QuorumOrganism>::AddOrgBirth(offspring, parent_pos));
    }

    unsigned int AddOrgBirth(QuorumOrganism * offspring, QuorumOrganism * parent) {
      return this->AddOrgBirth(offspring, parent->get_loc());
    }

    /// Does public good creation / distrubtion processing for an organism
    /// DOES NOT CHECK FOR NULL POINTERS
    void Publicize(QuorumOrganism * org) {
      auto neighbors = POP_MANAGER<QuorumOrganism>::GetOrgNeighbors(org->get_loc());
      auto cluster = POP_MANAGER<QuorumOrganism>::GetClusterByRadius(org->get_loc(),
                                                                    ai_radius);
      cluster.erase(org);
      int contribution;
      // get contribution and round-robin it out to the various orgs
      // producer gets first dibs
      if( (contribution = org->get_contribution(calculate_quorum(cluster))) > 0){
        // TODO: make this mor eefficient. Can easily math who gets who much (probably)
        auto recipiant = neighbors.end();
        
        // iterate over neighbors && give them things, with donator going first
        while(contribution > 0) {
          if (recipiant == neighbors.end()) {
            recipiant = neighbors.begin()--; // will be incremented at the end of the loop
            org->add_points(1);
          }
          else if ((*recipiant) != nullptr) {
            (*recipiant)->add_points(1);
          }
          contribution--;
          recipiant++;
        }
      }
    }

    // function to make offspring of this org; does mutation, if enabled
    QuorumOrganism * make_offspring(QuorumOrganism * parent) {
      QuorumOrganism * offspring = new QuorumOrganism(*parent);
      offspring->mutate();
      offspring->state.reset_age();
      offspring->state.reset_points();
      assert(offspring != parent);
      return offspring;
    }


    // function to handle reproduction of this organism
    // will return pointer to offspring if capable of reproducing
    // nullptr otherwise
    // will decrement the points needed to reproduce from state
    QuorumOrganism * reproduce(QuorumOrganism * parent) {
      if (parent->state.get_points() >= QuorumOrganism::needed_to_reproduce) {
        parent->state.reset_points();
        parent->state.reset_age();
        return make_offspring(parent);
      }

      return nullptr;
    }


    // function to iterate over the population && generate the next population.
    // will rely heavily on things implemented by the underlying structural class.
    // update works in stages; stage 1: determine who is co-operating
    // stage two: handle reproduction
    void Update() {

      // for each org get its contribution to neighbots
      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        if (org == nullptr) {continue;} // don't even try to touch nulls
        org->state.reset_accounting();
        if (available_private_points > 0) {
          org->add_points(1); // metabolize
          available_private_points--;
        }
        Publicize(org);
      }

      // now do reproduction
      // organism will decide if it can reproduce
      // if so, will yield a pointer to a mutated offspring
      // see QuorumOrg.h:199ish for deets
      QuorumOrganism * offspring = nullptr;
      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        if (org == nullptr) {continue;} // don't even try to touch nulls
        org->state.bump_age();
        // while able, gen offspring
        if ((offspring = reproduce(org)) != nullptr) {
          // overloaded AddOrgBirth will determine parent loc from pointer
          AddOrgBirth(offspring, org);
          auto uselessptr = offspring;
        }
      }
    } // end Update()

    
    void Print(std::function<std::string(QuorumOrganism *)> string_fun, std::ostream & os = std::cout,
              std::string empty="X", std::string spacer=" ") {
      os << "Still no." << std::endl;
    }

    // quick and dirty way to spit out the population
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      for (auto org : POP_MANAGER<QuorumOrganism>::pop) {
        if (org != nullptr) {os << "\t{" << *org << "} " << std::endl;}
      }
    }

  };


template <class QuorumOrganism, template<class> class POP_MANAGER>
int QuorumManager<QuorumOrganism, POP_MANAGER>::hi_weight;
template <class QuorumOrganism, template<class> class POP_MANAGER>
int QuorumManager<QuorumOrganism, POP_MANAGER>::lo_weight;
template <class QuorumOrganism, template<class> class POP_MANAGER>
int QuorumManager<QuorumOrganism, POP_MANAGER>::ai_radius;

template <class QuorumOrganism, template<class> class POP_MANAGER>
std::ostream & operator<< (std::ostream & out, QuorumManager<QuorumOrganism, POP_MANAGER> & qm) {
  qm.Print(out);
  return out;
}
} // close evo::
} // close emp::
#endif
