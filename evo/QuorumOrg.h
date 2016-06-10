//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains the general configuration for a quorum-sensing organism


#ifndef EMP_EVO_QUORUM_ORGANISM
#define EMP_EVO_QUORUM_ORGANISM

#include "../config/config.h"
#include "../tools/Random.h"
#include <iostream>

/* TODO: Implement the config stuff
 *    - # pts donated when co-operating
 * TODO: implement actual use of the org's id in the state object
 * TODO: impelement some kind of 'tick' processing function, to handle
 *     tracking cost of AI production && if co-operating
 */

/* Simple class to contain the genome for a QuorumOrg
 * Shoved in here in case we decide to use the fancy liniage tracker I saw
 * in this directory.
 *
 * Also because it'll be easy to make specific types of orgs.
 */


namespace emp {
namespace evo {

struct QuorumOrgGenome {
  double co_op_prob;
  double ai_radius;
  double quorum_threshold;

  QuorumOrgGenome () {
    co_op_prob = 0;
    ai_radius = 10;
    quorum_threshold = 1;
  }

  QuorumOrgGenome(double cprob, double airad, double qthresh) :
    co_op_prob(cprob), ai_radius(airad), quorum_threshold(qthresh) {};

  // prints co-op prob, ai_gen_prob, quorum_thresh
  void print (std::ostream & out) {
    out << co_op_prob << ", " << ai_radius << ", " << quorum_threshold;
  }

  std::ostream & operator << (std::ostream & out) {
    print(out);
    return out;
  }
};


std::ostream & operator<< (std::ostream & out, QuorumOrgGenome & genome) {
  genome.print(out);
  return out;
}

  /* Simple class to contain the current state of the organism
   * since everything is a primitive a bitwise copy for the constructor should work fine!
   * ...right?
   */

struct QuorumOrgState {
  QuorumOrgGenome genome;

  bool hi_density;
  bool mutate;    // dunno if we'll want to do this by-org, but eh

  int points;
  unsigned int age;
  unsigned int loc;
  unsigned int num_offspring; // gonna use this as/in a basic fitness function

  QuorumOrgState() {
    hi_density = false;
    mutate = false;
  };

  QuorumOrgState (double cprob, double aprob, double qthresh, bool mut, unsigned int pts) :
    genome(cprob, aprob, qthresh), mutate(mut), points(pts) {

    age = 0;
    loc = 0;
    num_offspring = 0;
    hi_density = false;
  };

  // prints out loc, age, points, if_producing_ai, if_mutation_on, {genome}
  void print (std::ostream & out) {
    out << loc << ", " << age << ", " << points << ", " << num_offspring;
    out << ", " << hi_density << ", " << mutate << ", (";
    //genome.print(out); // dunno why streaming doesn't work here
    out << genome;
    out << ")";
  }

};

// for some reason these have to be outside of the class to work properly
std::ostream & operator<< (std::ostream & out, QuorumOrgState & state) {
    state.print(out);
    return out;
}


/* Here we see the QuorumOrganism in its natural habitat....
 */

class QuorumOrganism {

public:
  static emp::Random * random;
  static int num_to_donate, needed_to_reproduce, cost_to_donate;
  static double mutation_amount;

  static const double initial_configurations[3][3];
  // access specifiers are really annoying. 
  QuorumOrgState state;
  QuorumOrganism () {};

  // Config constructor
  QuorumOrganism (double cprob, double airad, double qthresh, bool mut, unsigned int pts) {
    this->QuorumOrganism::state = QuorumOrgState(cprob, airad, qthresh, mut, pts);
  }

  // copy constructor; probably the primary way of reproduction will be to
  // copy the current org & call "mutate" on it
  QuorumOrganism (const QuorumOrganism & source) {
    // explicit is better than implicit...
    this->state = source.state;
    this->random = source.random;
  }

  // the aforementioned mutate function
  bool mutate (Random & random) {
    if (state.mutate) {
      //state.genome.quorum_threshold += random.GetRandNormal(0, MUTATION_AMOUNT);
      //state.genome.ai_radius += random.GetRandNormal(0, MUTATION_AMOUNT);
      state.genome.co_op_prob += random.GetRandNormal(0, mutation_amount);
      return true;
    }
    else {return false;}
  }


  bool mutate() {
    return mutate(*(this->random));
  }


  // utility/accessor methods
  void set_state (QuorumOrgState new_state) {state = new_state;}
  void increment_age() {state.age++;}
  unsigned int set_id (unsigned int new_id) {return state.loc = new_id;}
  unsigned int get_loc() {return state.loc;}
  unsigned int get_age() {return state.points;}
  unsigned int add_points(unsigned int points) {return state.points += points;}
  void set_density(bool hd) {state.hi_density = hd;}
  bool set_density(double q) { return state.hi_density = (q > state.genome.quorum_threshold);}
  bool hi_density () const {return state.hi_density;}
  unsigned int get_fitness() {return state.num_offspring;}

  // methods for interacting with the world / neighbors
  int get_contribution (double current_quorum) {
    set_density(current_quorum);
    if (random->P(state.genome.co_op_prob) &&
      current_quorum >= state.genome.quorum_threshold) {

      // gonna add a check to see if we contribute when we're poor
      // dunno if I should
      if (state.points >= cost_to_donate) {
        state.points -= cost_to_donate;
        return num_to_donate;
      }
    }
    return 0;
  }

  void print (std::ostream & out) {
    out << state;
  }

  // function to make offspring of this org; does mutation, if enabled
  QuorumOrganism * make_offspring() {
    QuorumOrganism * offspring = new QuorumOrganism(*this);
    offspring->mutate();
    return offspring;
  }

  // function to handle reproduction of this organism
  // will return pointer to offspring if capable of reproducing
  // nullptr otherwise
  // will decrement the points needed to reproduce from state
  QuorumOrganism * reproduce() {
    if (state.points >= needed_to_reproduce) {
      state.points -= needed_to_reproduce;
      state.num_offspring++;
      return make_offspring();
    }

    return nullptr;
  }

  bool operator<(const QuorumOrganism & other) const {
    return this->state.loc < other.state.loc;
  }

  bool operator>(const QuorumOrganism & other) const {
    return this->state.loc > other.state.loc;
  }

  bool operator==(const QuorumOrganism & other) const {
    return this->state.loc == other.state.loc;
  }

};

std::ostream & operator << (std::ostream & out, QuorumOrganism & org) {
   org.print(out);
   return out;
}


/// selection of standardized starting configurations for QOrgs
const double QuorumOrganism::initial_configurations[3][3]  = {
    {0.5, 10, 1}, // Standard
    {0, 10, 1}, // defector
    {1, 10, 1} // co-operator
};

int QuorumOrganism::num_to_donate;
int QuorumOrganism::cost_to_donate;
int QuorumOrganism::needed_to_reproduce;
double QuorumOrganism::mutation_amount;
emp::Random * QuorumOrganism::random;

} // close evo::
} // close emp::
#endif
