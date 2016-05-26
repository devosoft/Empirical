// silly test file that should never make it into a commit
//
// this should eventually find its way into the testing infastructure
// TODO: add to testing infastructure

#include "QuorumOrg.h"
#include "QuorumManager.h"
#include "World.h"

#include <iostream>

using QOrg = emp::evo::QuorumOrganism;

template <template<class> class BASE_PM>
using QM = emp::evo::QuorumManager<QOrg, BASE_PM>;

template <class QOrg, template<class> class BASE_PM>
using QWorld = emp::evo::World<QOrg, QM<BASE_PM>>;

//TODO: find a way to enforce that POP_MANAGER is POP_MANAGER<QOrg>
// Consult Emily's stats stuff for reference...?


int main() {
    emp::Random dice;

    // test reproduction
    QOrg Simon(.5, .5, .5, true, 0, &dice);

    int num_ticks = 0;
    QOrg * SimonJr = nullptr;

    do {
        SimonJr = Simon.reproduce();
        num_ticks++;
        Simon.add_points(1);
    } while (SimonJr == nullptr);

    SimonJr->set_id(2);

    std::cout << Simon << std::endl;
    std::cout << *SimonJr << " (after " << num_ticks - 1<< " ticks; should be 10)." << std::endl;
    // require offspring to be created after 10 ticks

    // test AI generating organism
    QOrg Samaritan(.5, 1, .5, true, 5, &dice);
    std::cout << "Samaritan's AI production is " << Samaritan.making_ai();
    std::cout << " (should be true/1)" << std::endl;
    // require to be true/1

    // test forced co-operation
    QOrg BetterSamaritan(1, 1, .5, true, 3, &dice);
    std::cout << "BetterSamaritan is doinating " << BetterSamaritan.get_contribution(.6);
    std::cout << " points (should be 0)." << std::endl;

    // make it able, but keep below quorum
    BetterSamaritan.add_points(7);
    std::cout << "BetterSamaritan is doinating " << BetterSamaritan.get_contribution(.4);
    std::cout << " points (should be 0)." << std::endl;

    std::cout << "BetterSamaritan is doinating " << BetterSamaritan.get_contribution(.6);
    std::cout << " points (should be 5)." << std::endl;
    std::cout << "State is: " << BetterSamaritan << " (should be ";
    std::cout << " '0, 0, 5, 1, 1, (1, 1, 0.5)' " << std::endl;


    // now to test the actual population manager.
    // note: this will currently ONLY work with the grid pop as a base,
    // since only it has the get_org_neighbors() funciton impelemented.

    // I'm using recplica.cc from jgf_learning as a reference for these tests
    //TODO: test integration with stats managers
    const unsigned int pop_size = 30;
    const unsigned int runtime = 100;
    QWorld<QOrg, emp::evo::PopulationManager_Grid> Qpop(&dice);

    std::function<double(QOrg * )>fit_func=[](QOrg* org) {
      if (org != nullptr ) {return (double) org->get_fitness();}
      else {return (double) -1;} // seriously, I have to cast this to make you happy?
    };

    // build random initial Population
    for(unsigned int i = 0; i < pop_size; i++) {
      QOrg * org = new QOrg(.5, .5, .5, true, 0, &dice);
      org->force_mutation();
      Qpop.Insert(*org);
    }

    // mutation is handled automatically by the population QPop_Manager, currently
    // I'm getting the sense that it probably shouldn't be.
    Qpop.SetDefaultMutateFun([](QOrg * org, emp::Random & random) {
      return org->mutate(random);
    });

    // loop through the specified number of updates && run the evolution
    for (unsigned int update_num = 0; update_num < runtime; update_num++) {
      Qpop.TournamentSelect(fit_func, 10, pop_size);
      Qpop.Update();
    }

    // print the population
    std::cout << "Final population: " << std::endl;
    //std::cout << Qpop << std::endl;
    Qpop.Print(std::cout, "", "");

}
