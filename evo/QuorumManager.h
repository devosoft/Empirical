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
    
    /// calculates quorum and updates state.hi_density  
    double calculate_quorum (std::set<QuorumOrganism *> neighbors) {
      double active_neighbors = 0;
      //unsigned int total_neighbors = 0;

      for (auto org_iter : neighbors) {
        if (org_iter == nullptr) {continue;} // ignore nonextant orgs
        if ( org_iter->hi_density()) {active_neighbors += hi_weight;}
        else {active_neighbors += lo_weight;}
      }

      return active_neighbors;
    }

    double calculate_quorum (QuorumOrganism * org) {
      double result;    
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

    // minor override to the parent class to save the orgs location to the org
    unsigned int AddOrg (QuorumOrganism * org) {
      return org->set_id(POP_MANAGER<QuorumOrganism>::AddOrg(org));
    }

    unsigned int AddOrgBirth (QuorumOrganism * offspring, int parent_pos) {
      return offspring->set_id(POP_MANAGER<QuorumOrganism>::AddOrgBirth(offspring, parent_pos));
    }

    unsigned int AddOrgBirth(QuorumOrganism * offspring, QuorumOrganism * parent) {
      return AddOrgBirth(offspring, parent->get_loc());
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
            recipiant = neighbors.begin();
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

    // function to iterate over the population && generate the next population.
    // will rely heavily on things implemented by the underlying structural class.
    // update works in stages; stage 1: determine who is co-operating
    // stage two: handle reproduction
    void Update() {

      // for each org get its contribution to neighbots
      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        if (org == nullptr) {continue;} // don't even try to touch nulls
        org->add_points(1); // metabolize
        Publicize(org);
      }

      // now do reproduction
      // organism will decide if it can reproduce
      // if so, will yield a pointer to a mutated offspring
      // see QuorumOrg.h:199ish for deets
      QuorumOrganism * offspring = nullptr;
      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        if (org == nullptr) {continue;} // don't even try to touch nulls
        if( (offspring = org->reproduce()) != nullptr) {
          // overloaded AddOrgBirth will determine parent loc from pointer
          AddOrgBirth(offspring, org);
        }
        org->increment_age();
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
