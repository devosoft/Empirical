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

bool operator== (QuorumOrgGenome const & lhs, QuorumOrgGenome const & rhs) {
  return (lhs.co_op_prob == rhs.co_op_prob &&
          lhs.ai_radius == rhs.ai_radius &&
          lhs.quorum_threshold == rhs.quorum_threshold);
}


std::ostream & operator<< (std::ostream & out, QuorumOrgGenome & genome) {
  genome.print(out);
  return out;
}

  /* Simple class to contain the current state of the organism
   */

struct QuorumOrgState {
protected:
  unsigned int age;
  int points;

public:
  QuorumOrgGenome genome;

  bool hi_density;
  bool mutate;    // dunno if we'll want to do this by-org, but eh

  unsigned int loc;
  unsigned int num_offspring; // gonna use this as/in a basic fitness function

  unsigned int get_age() const {return age;}
  unsigned int get_points() const {return points;}
  unsigned int add_points(unsigned int add) {return points += add;}
  void reset_points() {points = 0;}
  unsigned int remove_points(unsigned int remove) {
    if (remove <= points) {return points -= remove;}
    else return points;
  }
  unsigned int bump_age() {return ++age;}
  void reset_age() {age = 0;}

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

  // Copy constructor!
  QuorumOrgState (const QuorumOrgState & other) : genome(other.genome.co_op_prob,
                                                  other.genome.ai_radius,
                                                  other.genome.quorum_threshold),
                                                  hi_density(false), mutate(other.mutate), 
                                                  points(0), age(0), loc(0), num_offspring(0) {
  }

  QuorumOrgState operator= (const QuorumOrgState & rhs) {
    age = rhs.age;
    loc = rhs.loc;
    num_offspring = rhs.num_offspring;
    hi_density = rhs.hi_density;
    genome = rhs.genome;
    points = rhs.points;
    return *this;
  }

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

  static const double initial_configurations[4][3];
  // access specifiers are really annoying. 
  QuorumOrgState state;
  QuorumOrganism () {};

  // Config constructor
  QuorumOrganism (double cprob, double airad, double qthresh, bool mut, unsigned int pts) {
    this->QuorumOrganism::state = QuorumOrgState(cprob, airad, qthresh, mut, pts);
  }

  QuorumOrganism (const QuorumOrganism & other) {
    this->state = QuorumOrgState(other.state);
    this->random = other.random;
  }

  QuorumOrganism & operator=(const QuorumOrganism & other) {
    if (&other == this) {return *this;}
    this->state = *(new QuorumOrgState(other.state));
    this->random = other.random;
    return *this;
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
  unsigned int set_id (unsigned int new_id) {return state.loc = new_id;}
  unsigned int get_loc() {return state.loc;}
  unsigned int get_age() {return state.get_age();}
  unsigned int add_points(unsigned int points) {return state.add_points(points);}
  unsigned int get_points() {return state.get_points();}
  void set_density(bool hd) {state.hi_density = hd;}
  bool set_density(double q) { return state.hi_density = (q > state.genome.quorum_threshold);}
  bool hi_density () const {return state.hi_density;}
  unsigned int get_fitness() {return state.get_points();}

  // methods for interacting with the world / neighbors
  int get_contribution (double current_quorum) {
    set_density(current_quorum);
    if (random->P(state.genome.co_op_prob) && current_quorum >= state.genome.quorum_threshold) {
      if (state.get_points() >= cost_to_donate) {
        state.remove_points(cost_to_donate);
        return num_to_donate;
      }
  
    }
    return 0;
  }

  void print (std::ostream & out) {
    out << state;
  }

  bool operator<(const QuorumOrganism & other) const {
    return this->state.loc < other.state.loc;
  }

  bool operator>(const QuorumOrganism & other) const {
    return this->state.loc > other.state.loc;
  }

  bool operator==(const QuorumOrganism & other) const {
    return this->state.genome == other.state.genome;
  }

};

std::ostream & operator << (std::ostream & out, QuorumOrganism & org) {
   org.print(out);
   return out;
}


/// selection of standardized starting configurations for QOrgs
const double QuorumOrganism::initial_configurations[4][3]  = {
    {0.5, 10, 1}, // Standard
    {0, 10, 1}, // defector
    {1, 10, 1},// donator (effectively sterile)
    {0.015, 10, 1} 
};

int QuorumOrganism::num_to_donate;
int QuorumOrganism::cost_to_donate;
int QuorumOrganism::needed_to_reproduce;
double QuorumOrganism::mutation_amount;
emp::Random * QuorumOrganism::random;

} // close evo::
} // close emp::
#endif
