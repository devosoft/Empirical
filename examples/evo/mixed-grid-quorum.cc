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
#include <iomanip>
#include <string>
#include <fstream>

#include "quorum.cc"

// define the underlying population manager here
template <class QOrg>
using FOUNDATION = emp::evo::PopulationManager_MixedGrid<QOrg>;

int main (int argc, char * argv[]) {
  std::string prefix;

  auto conf = get_config(argc, argv, prefix);
  auto state = QuorumRunState<FOUNDATION>(*conf, prefix);

  state.Qmapper = new emp::evo::StatsManager_Mapper<QM<FOUNDATION>>(state.Qpop, 
                                                                      conf->GRID_X(),
                                                                      conf->GRID_Y(),
                                                                      prefix + "quorum-map.csv");


  state.Qpop->ConfigPop(conf->GRID_X(), conf->GRID_Y());
  configure_stats_manager(state);
  
  return execute<FOUNDATION>(state);  
}
