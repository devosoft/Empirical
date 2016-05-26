//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains the  manager for quroum organisms

#ifndef EMP_EVO_QUORUM_MANAGER
#define EMP_EVO_QUORUM_MANAGER

#include "../tools/Random.h"
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

    // we require that the base class has a get_neighbors methods

    // function to calculate if the given organism is at quorum
    // more accurately, it determines % of neighbors emitting the signal
    // let's assume the base class implements get_org_neighbors() that returns
    // an emp_vector of org*'s'
    // or array, or other iterable container--doesn't matter
    // TODO: determine if this should/does include the target org in quorum calc

  protected:
    double calculate_quorum (std::set<QuorumOrganism *> neighbors) {
      unsigned int active_neighbors = 0;
      unsigned int total_neighbors = 0;

      for (auto org_iter : neighbors) {

        if ( org_iter != nullptr && org_iter->making_ai()) {active_neighbors++;}
        total_neighbors++; // could probably get this from neighbors.size() or something
      }

      return (double) active_neighbors / (double) total_neighbors;
    }

    double calculate_quorum (QuorumOrganism * org) {
      // for an example of an implemented get_org_neighbors see Grid pop manager.
      // PopulationManager.h:209ish
      auto neighbors = POP_MANAGER<QuorumOrganism>::get_org_neighbors(org->get_loc());

      return calculate_quorum(neighbors);
    }

public:
    // minor override to the parent class to save the orgs location to the org
    unsigned int AddOrg (QuorumOrganism * org) {
      return org->get_loc() = POP_MANAGER<QuorumOrganism>::AddOrg(org);
    }

    unsigned int AddOrgBirth (QuorumOrganism * offspring, int parent_pos) {
      return offspring->get_loc() = POP_MANAGER<QuorumOrganism>::AddOrgBirth(offspring, parent_pos);
    }

    unsigned int AddOrgBirth(QuorumOrganism * offspring, QuorumOrganism * parent) {
      return AddOrgBirth(offspring, parent->state.loc);
    }

    // function to iterate over the population && generate the next population.
    // will rely heavily on things implemented by the underlying structural class.
    // update works in stages; stage 1: determine who is co-operating
    // stage two: handle reproduction
    void Update() {

      // for each org get its contribution to neighbots
      int contribution;

      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        auto neighbors = POP_MANAGER<QuorumOrganism>::get_org_neighbors(org->state.loc);
        if( (contribution = org->get_contribution(calculate_quorum(neighbors)) > 0)){
          for (QuorumOrganism * neigh : neighbors) {neigh->add_points(contribution);}
        }
        org->add_points(1); // metabolize
      }

      // now do reproduction
      // organism will decide if it can reproduce
      // if so, will yield a pointer to a mutated offspring
      // see QuorumOrg.h:199ish for deets
      QuorumOrganism * offspring = nullptr;
      for(QuorumOrganism * org : POP_MANAGER<QuorumOrganism>::pop) {
        if( (offspring = org->reproduce()) != nullptr) {
          // overloaded AddOrgBirth will determine parent loc from pointer
          AddOrgBirth(offspring, org);
        }
      }
    } // end Update()

    // quick and dirty way to spit out the population
    void Print(std::ostream & out) {
      for (auto org : POP_MANAGER<QuorumOrganism>::pop) {
        if (*org != nullptr) {out << "\t{" << *(*org) << "} " << std::endl;}
      }
    }

    void Print(std::function<std::string(QuorumOrganism *)> string_fun, std::ostream & os = std::cout,
              std::string empty="X", std::string spacer=" ") {
      os << "Still no." << std::endl;
    }
    void Print(std::ostream & os = std::cout, std::string empty="X", std::string spacer=" ") {
      os << "No." << std::endl;
    }

  };

template <class QuorumOrganism, template<class> class POP_MANAGER>
std::ostream & operator<< (std::ostream & out, QuorumManager<QuorumOrganism, POP_MANAGER> & qm) {
  qm.Print(out);
  return out;
}
} // close evo::
} // close emp::
#endif
