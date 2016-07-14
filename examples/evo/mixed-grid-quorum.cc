//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file demonstrates the functionality of QuorumOrganisms and their associated
//  subsystems

#include <iostream>
#include <string>

#include "../../evo/StatsManager.h"
#include "../../evo/EvoStats.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

#include "quorum.cc"

using QuorumDriverConfig = QuorumConfigBase;

// define the underlying population manager here
template <class QOrg>
using FOUNDATION = emp::evo::PopulationManager_MixedGrid<QOrg>;

int main (int argc, char * argv[]) {
  std::string prefix;

  auto conf = get_config<QuorumDriverConfig>(argc, argv, prefix);
  auto state = QuorumRunState<FOUNDATION, QuorumDriverConfig>(*conf, prefix);

  state.Qmapper = new emp::evo::StatsManager_Mapper<QM<FOUNDATION>>(state.Qpop, 
                                                                      conf->GRID_X(),
                                                                      conf->GRID_Y(),
                                                                      prefix + "quorum-map.csv");


  state.Qpop->ConfigPop(conf->GRID_X(), conf->GRID_Y());
  configure_stats_manager<FOUNDATION, QuorumDriverConfig>(state);
  
  return execute<FOUNDATION, QuorumDriverConfig>(state);  
}
