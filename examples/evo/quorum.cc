//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file demonstrates the functionality of QuorumOrganisms and their associated
//  subsystems

#include <iostream>
#include <string>

#include "../../config/ArgManager.h"
#include "../../evo/World.h"
#include "../../tools/Random.h"
#include "../../evo/EvoStats.h"
#include "../../evo/StatsManager.h"
#include "../../evo/QuorumOrg.h"
#include "../../evo/QuorumManager.h"

#include <sstream>
#include <iostream>

using QOrg = emp::evo::QuorumOrganism;

template <template<class> class BASE_PM>
using QM = emp::evo::QuorumManager<QOrg, BASE_PM>;

template <class QOrg, template<class> class BASE_PM>
using QWorld = emp::evo::World<QOrg, QM<BASE_PM>>;

//TODO: find a way to enforce that POP_MANAGER is POP_MANAGER<QOrg>
// Consult Emily's stats stuff for reference...?

EMP_BUILD_CONFIG( QuorumConfig,
    VALUE(HI_AI_WEIGHT, int, 4, "What value should the AI production be for hi-density?"),
    VALUE(LO_AI_WEIGHT, int, 1, "What value should the AI production be for lo-density?"),
    VALUE(AI_RADIUS, int, 10, "What's the radius of AI dispersal?"),
    VALUE(NUM_TO_DONATE, int, 45, "Number of points a public good is 'worth'"),
    VALUE(NEEDED_TO_REPRODUCE, int, 50, "Number of points needed for an organism to reproduce"),
    VALUE(COST_TO_DONATE, int, 25, "Number of points a public good costs to produce"),
    VALUE(MUTATION_AMOUNT, double, 0.1, "Standard deviation of distribution to mutate by"),
    VALUE(GRID_X, int, 60, "Width of the grid"),
    VALUE(GRID_Y, int, 60, "Height of the grid"),
    VALUE(TICKS, int, 1000, "Length of simulation"),
    VALUE(INITIAL_SIZE, unsigned int, 30, "Starting population size"),
    VALUE(INITIAL_CONFIG, int, 0, "Index of the QOrg initial config array to use as inital config"),
    VALUE(ENABLE_MUTATION, bool, true, "If mutation should be enabled"),
    VALUE(RAND_SEED, int, 238947, "Seed for the random generator")
)


int main(int argc, char* argv[]) {
    
    QuorumConfig config;
    config.Read("quorum.cfg");

    emp::Random dice;
    std::string prefix;

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(config, std::cout, "quorum.cfg") == false) {exit(0);}
    if (args.TestUnknown() == false) {exit(0);}

    if (argc == 1) { // program name, no prefix
      prefix = "";
    }
    else if (argc == 2) { // has prefix
      prefix = std::string(argv[1]) + "-";
    }
    else {
      std::cerr << "** Usage: ./quorum output-prefix";
    }


    QWorld<QOrg, emp::evo::PopulationManager_Grid> Qpop(&dice);
    Qpop.ConfigPop(config.GRID_X(), config.GRID_Y());

    emp::evo::StatsManager_DefaultStats<emp::evo::QuorumManager<QOrg, emp::evo::PopulationManager_Grid>> Qstats(&Qpop, prefix + "quorum.csv"); 

    // Set all the class variables
    QM<emp::evo::PopulationManager_Grid>::hi_weight = config.HI_AI_WEIGHT();
    QM<emp::evo::PopulationManager_Grid>::lo_weight = config.LO_AI_WEIGHT();
    QM<emp::evo::PopulationManager_Grid>::ai_radius = config.AI_RADIUS();
    QOrg::num_to_donate = config.NUM_TO_DONATE();
    QOrg::needed_to_reproduce = config.NEEDED_TO_REPRODUCE();
    QOrg::cost_to_donate = config.COST_TO_DONATE();
    QOrg::mutation_amount = config.MUTATION_AMOUNT();
    QOrg::random = &dice; // our random is gotten from the world
    unsigned int runtime = config.TICKS();
    unsigned int pop_size = config.INITIAL_SIZE();
    dice.ResetSeed(config.RAND_SEED());

    config.Write("quorum.cfg");

        // build random initial Population
    for(unsigned int i = 0; i < pop_size; i++) {
      QOrg * org = new QOrg(QOrg::initial_configurations[config.INITIAL_CONFIG()][0], 
                            QOrg::initial_configurations[config.INITIAL_CONFIG()][1], 
                            QOrg::initial_configurations[config.INITIAL_CONFIG()][2], 
                            config.ENABLE_MUTATION(), 0);
      org->mutate();
      Qpop.Insert(*org);
    }

    // mutation is handled automatically by the population QPop_Manager, currently
    // I'm getting the sense that it probably shouldn't be.
    Qpop.SetDefaultMutateFun([](QOrg * org, emp::Random & random) {
      return org->mutate(random);
    });

    std::function<double(QOrg * )>fit_func=[](QOrg* org) {
      if (org != nullptr ) {return (double) org->get_fitness();}
      else {return (double) -1;} // seriously, I have to cast this to make you happy?
    };

    std::function<double(QM<emp::evo::PopulationManager_Grid> *)>age_func=[]
      (QM<emp::evo::PopulationManager_Grid> * m) {
      double age = 0;
      int num_orgs = 0;
      for (auto org : (*m)) {
        if (org != nullptr) {
          age += org->get_age();
          num_orgs++;
        }
      }
      if (num_orgs == 0) {return (double) -1;}
      return age / (double) num_orgs;
    };

    Qstats.AddFunction(age_func, "Average age");

    Qpop.SetDefaultFitnessFun(fit_func);
    Qstats.SetDefaultFitnessFun(fit_func); 

    // loop through the specified number of updates && run the evolution
    for (unsigned int update_num = 0; update_num < runtime; update_num++) {
      Qpop.TournamentSelect(fit_func, 10, pop_size);
      Qpop.Update();
    }
}
