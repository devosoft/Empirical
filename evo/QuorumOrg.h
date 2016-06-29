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
  int lineage = -1;
  double co_op_prob;
  double ai_radius;
  double quorum_threshold;
  bool can_make_HiAI;
  bool can_make_LoAI;

  QuorumOrgGenome () {
    co_op_prob = 0;
    ai_radius = 10;
    quorum_threshold = 1;
    can_make_HiAI = false;
    can_make_LoAI = false;
  }

  QuorumOrgGenome(double cprob, double airad, double qthresh, int lin, bool cmha, bool cmla) :
    co_op_prob(cprob), ai_radius(airad), quorum_threshold(qthresh), lineage(lin),
    can_make_HiAI(cmha), can_make_LoAI(cmla) {};

  // prints co-op prob, ai_gen_prob, quorum_thresh
  void print (std::ostream & out) {
    out << co_op_prob << ", " << ai_radius << ", " << quorum_threshold;
  }

  std::ostream & operator << (std::ostream & out) {
    print(out);
    return out;
  }

  int get_lineage() const {return lineage;}
};

bool operator== (QuorumOrgGenome const & lhs, QuorumOrgGenome const & rhs) {
  return (lhs.co_op_prob == rhs.co_op_prob &&
          lhs.ai_radius == rhs.ai_radius &&
          lhs.quorum_threshold == rhs.quorum_threshold &&
          lhs.can_make_HiAI == rhs.can_make_HiAI && 
          lhs.can_make_LoAI == rhs.can_make_LoAI);
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
  int out, in;

public:
  QuorumOrgGenome genome;

  bool hi_density;
  bool mutate;    // dunno if we'll want to do this by-org, but eh

  unsigned int loc;
  unsigned int num_offspring; // gonna use this as/in a basic fitness function

  unsigned int get_age() const {return age;}
  unsigned int get_points() const {return points;}
  unsigned int add_points(unsigned int add) {
    in += add;
    return points += add;
  }
  void reset_points() {points = 0;}
  unsigned int remove_points(unsigned int remove) {
    if (remove <= points) {
      out += points;
      return points -= remove;
    }
    else return points;
  }
  unsigned int bump_age() {return ++age;}
  void reset_age() {age = 0;}
  void reset_accounting() {out = 0; in = 0;}

  QuorumOrgState() {
    hi_density = false;
    mutate = false;
  };

  QuorumOrgState (double cprob, double aprob, double qthresh, bool mut, 
                  unsigned int pts, int lin=-1, bool cmha = false, bool cmla = false) :
    genome(cprob, aprob, qthresh, lin, cmha, cmla), mutate(mut), points(pts) {
 
    age = 0;
    loc = 0;
    num_offspring = 0;
    hi_density = false;
  };

  // Copy constructor!
  QuorumOrgState (const QuorumOrgState & other) : genome(other.genome.co_op_prob,
                                                  other.genome.ai_radius,
                                                  other.genome.quorum_threshold,
                                                  other.genome.get_lineage(),
                                                  other.genome.can_make_HiAI,
                                                  other.genome.can_make_LoAI),
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
    mutate = rhs.mutate;
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
  static unsigned int num_to_donate, needed_to_reproduce, cost_to_donate;
  static double mutation_amount;

  static const QuorumOrgGenome initial_configurations[5];

  static int classify (QuorumOrganism const * org) {
    if (org == nullptr) { return -1;}
    return (int) org->state.genome.co_op_prob * 10;
  }
  
  // access specifiers are really annoying. 
  QuorumOrgState state;
  QuorumOrganism () {};

  // Config constructor
  QuorumOrganism (double cprob, double airad, double qthresh, bool mut, 
                  unsigned int pts, int lin, bool cmha, bool cmla) {
    this->QuorumOrganism::state = QuorumOrgState(cprob, airad, qthresh, mut, pts, lin, cmha, cmla);
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
  void set_density(bool hd) {state.hi_density = hd && state.genome.can_make_HiAI;}
  bool set_density(double q) { 
    state.hi_density = (q > state.genome.quorum_threshold);
    return state.hi_density = state.hi_density && state.genome.can_make_HiAI;
  }
  bool hi_density () const {return state.hi_density && state.genome.can_make_HiAI;}
  bool lo_density () const {return state.genome.can_make_LoAI;}
  unsigned int get_fitness() {return state.get_points();}

  // methods for interacting with the world / neighbors
  int get_contribution (bool current_quorum) {
    set_density(current_quorum);
    if (random->P(state.genome.co_op_prob) && current_quorum && state.hi_density) {
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



const QuorumOrgGenome standard_genome = QuorumOrgGenome(0.5, 10, 40, 0, true, true);
const QuorumOrgGenome lying_defector_genome = QuorumOrgGenome(0, 10, 40, 1, false, true);
const QuorumOrgGenome cooperator_genome = QuorumOrgGenome(1, 10, 40, 2, true, true);
const QuorumOrgGenome scrooge_genome = QuorumOrgGenome(0.015, 10, 40, 3, true, true);
const QuorumOrgGenome truthful_defector_genome = QuorumOrgGenome(0, 10, 40, 4, false, false);


/// selection of standardized starting configurations for QOrgs
const QuorumOrgGenome QuorumOrganism::initial_configurations[5] = {
  standard_genome,
  lying_defector_genome,
  cooperator_genome,
  scrooge_genome,
  truthful_defector_genome
};

unsigned int QuorumOrganism::num_to_donate;
unsigned int QuorumOrganism::cost_to_donate;
unsigned int QuorumOrganism::needed_to_reproduce;
double QuorumOrganism::mutation_amount;
emp::Random * QuorumOrganism::random;

} // close evo::
} // close emp::
#endif
