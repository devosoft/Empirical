//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains the general configuration for a quorum-sensing organism


#ifndef EMP_EVO_QUORUM_ORGANISM
#define EMP_EVO_QUORUM_ORGANISM

#include "../tools/Random.h"
#include <iostream>

/* TODO: Implement the config stuff
 *      - # pts donated when co-operating
 * TODO: implement actual use of the org's id in the state object
 */

/* Simple class to contain the genome for a QuorumOrg
 * Shoved in here in case we decide to use the fancy liniage tracker I saw
 * in this directory.
 *
 * Also because it'll be easy to make specific types of orgs.
 */


// constants that will eventually be configurable
const unsigned int NUM_TO_DONATE = 5;
const double MUTATION_AMOUNT = 0.1;
const double MUTATION_RATE = 0.005;


namespace emp {
namespace evo {


/* Here we see the QuorumOrganism in its natural habitat....
 * 
 * 
 */

class QuorumOrganism {

    struct QuorumOrgGenome {
        double co_op_prob;
        double ai_gen_prob;
        double quorum_threshold;

        QuorumOrgGenome () {
            co_op_prob = 0;
            ai_gen_prob = 0;
            quorum_threshold = 1.1;
        }

        QuorumOrgGenome(double cprob, double aprob, double qthresh) :
            co_op_prob(cprob), ai_gen_prob(aprob), quorum_threshold(qthresh) {};

        // prints co-op prob, ai_gen_prob, quorum_thresh
        void print (std::ostream & out) {
            out << co_op_prob << ", " << ai_gen_prob << ", " << quorum_threshold;
        }

        std::ostream & operator << (std::ostream & out) {
            print(out);
            return out;
        }
    };


    /* Simple class to contain the current state of the organism
     * since everything is a primitive a bitwise copy for the constructor should work fine!
     * ...right?
     */

    struct QuorumOrgState {
        QuorumOrgGenome genome;
        
        bool producing_ai;
        bool mutate;        // dunno if we'll want to do this by-org, but eh
        
        unsigned int points;
        unsigned int age;
        unsigned int loc;

        QuorumOrgState() {
            producing_ai = false;
            mutate = false;
        };

        QuorumOrgState (double cprob, double aprob, double qthresh, bool mut, unsigned int pts) : 
            genome(cprob, aprob, qthresh), mutate(mut), points(pts) {
            
            age = 0;
            loc = 0;
        };

        // prints out loc, age, points, if_producing_ai, if_mutation_on, {genome}
        void print (std::ostream & out) {
            out << loc << ", " << age << ", " << points << ", " << producing_ai;
            out << ", " << mutate << ", (";
            genome.print(out); // dunno why streaming doesn't work here
            out << ")";
        }

    };

protected:
    QuorumOrgState state;
    emp::Random * random;

public:
    // Default Constructor
    QuorumOrganism () {};

    // Config constructor
    QuorumOrganism (double cprob, double aprob, double qthresh, bool mut, unsigned int pts,
                    emp::Random * rand) {
        this->QuorumOrganism::state = QuorumOrgState(cprob, aprob, qthresh, mut, pts);
        this->random = rand;
        this->state.producing_ai = random->P(aprob);  
    }

    // copy constructor; probably the primary way of reproduction will be to 
    // copy the current org & call "mutate" on it
    QuorumOrganism (const QuorumOrganism & source) {
        // explicit is better than implicit...
        this->state = source.state;
        this->random = source.random;
    }

    // the aforementioned mutate function
    bool mutate () {
        if (!state.mutate) {return false;} // if mutation is disabled fail out
        bool flag = false;
        if (random->P(MUTATION_RATE)) {
            state.genome.quorum_threshold += random->GetRandNormal(0, MUTATION_AMOUNT);
            flag = true;
        } if (random->P(MUTATION_RATE)) {
            state.genome.ai_gen_prob += random->GetRandNormal(0, MUTATION_AMOUNT);
            flag = true;
        } if (random->P(MUTATION_RATE)) {
            state.genome.co_op_prob += random->GetRandNormal(0, MUTATION_AMOUNT);
            flag = true;
        } // that felt silly

        return flag;
    }

    // utility/accessor methods
    void set_state (QuorumOrgState new_state) {state = new_state;}
    unsigned int add_points(unsigned int points) {return state.points += points;}
    bool making_ai () const {return state.producing_ai;}

    // methods for interacting with the world / neighbors
    int get_contribution (double current_quorum) {
        if (random->P(state.genome.co_op_prob) &&
            current_quorum <= state.genome.quorum_threshold) {

            // gonna add a check to see if we contribute when we're poor
            // dunno if I should
            if (state.points >= NUM_TO_DONATE) {
                state.points -= NUM_TO_DONATE;
                return NUM_TO_DONATE;
            }
        }
        return 0;
    }

    void print (std::ostream & out) {
        
    }

    std::ostream & operator << (std::ostream & out) {
       print(out);
       return out;
    }
};

} // close evo::
} // close emp::
#endif
