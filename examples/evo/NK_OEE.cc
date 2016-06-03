//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h


#include "../../web/web_init.h"
#include "../../web/Animate.h"
#include "../../emtools/JSWrap.h"

#include "../../evo/visualization_utils.h"
#include "../../../d3-emscripten/selection.h"
#include "../../../d3-emscripten/scales.h"
#include "../../../d3-emscripten/axis.h"


#include <iostream>
#include <functional>

#include "../../evo/NK-const.h"
#include "../../evo/OEE.h"
#include "../../evo/World.h"
#include "../../tools/BitSet.h"
#include "../../tools/Random.h"
#include "../../evo/StatsManager.h"

constexpr int N = config.N();
constexpr int K = config.K();

constexpr int POP_SIZE = config.POP_SIZE();
constexpr int UD_COUNT = config.MAX_GENS();
constexpr int TOURNAMENT_SIZE = config.TOURNAMENT_SIZE();

using BitOrg = emp::BitSet<N>;
FitnessVisualization<BitOrg > viz;


extern "C" int main()
{

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (int j = 0; j < N; j++) next_org[j] = viz.info.r->P(0.5);
    viz.info.world->Insert(next_org);
  }

  viz.info.world->SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );

  viz.Setup();
  viz.info.anim.Start();

}
