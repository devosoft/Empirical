//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h


#include "../../web/web_init.h"
#include "../../web/Animate.h"
#include "../../emtools/JSWrap.h"
#include "../../web/canvas_utils.h"
#include "../../web/web.h"
#include "../../web/Document.h"
#include "../../emtools/emfunctions.h"

#include "../../evo/visualization_utils.h"
#include "../../evo/evo_animation.h"
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
emp::web::GraphVisualization graph;
NKAnimation<BitOrg, emp::evo::DefaultStats> viz;

emp::web::Document doc("emp_base");

extern "C" int main()
{

  // Build a random initial population
  for (int i = 0; i < POP_SIZE; i++) {
    BitOrg next_org;
    for (int j = 0; j < N; j++) next_org[j] = viz.r->P(0.5);
    viz.world->Insert(next_org);
  }

  viz.world->SetDefaultMutateFun( [](BitOrg* org, emp::Random& random) {
      (*org)[random.GetInt(N)] = random.P(0.5);
      return true;
    } );

  graph.SetupConfigs(config);
  viz.world->statsM.ConnectVis(&graph);

  doc << "<br>";
  doc.AddButton([](){
      viz.anim.ToggleActive();
      auto but = doc.Button("toggle");
      if (viz.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Pause", "toggle");
  doc << graph;
  viz.anim.Start();

}
