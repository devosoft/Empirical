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
emp::web::GraphVisualization change_graph("Change", 550, 300);
emp::web::GraphVisualization novelty_graph("Novelty", 550, 300);
emp::web::GraphVisualization ecology_graph("Ecology", 550, 300);
emp::web::GraphVisualization complexity_graph("Complexity", 550, 300);
emp::web::LineageVisualization lineage_viz(5000, 300);
NKAnimation<BitOrg, emp::evo::LineagePruned, emp::evo::OEEStats> viz;

emp::web::Document doc("emp_base");

int main()
{


  //change_graph.SetupConfigs(config);
  viz.world->statsM.ConnectVis(&change_graph);
  //novelty_graph.SetupConfigs(config);
  viz.world->statsM.ConnectVis(&novelty_graph);
  //ecology_graph.SetupConfigs(config);
  viz.world->statsM.ConnectVis(&ecology_graph);
  //complexity_graph.SetupConfigs(config);
  viz.world->statsM.ConnectVis(&complexity_graph);
  viz.world->statsM.ConnectVis(&lineage_viz);
  viz.world->lineageM.ConnectVis(&lineage_viz);

  doc << "<br>";
  doc.AddButton([](){
      viz.anim.ToggleActive();
      auto but = doc.Button("toggle");
      if (viz.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Start", "toggle");
  doc << "<br>";
  doc << "<br>";
  doc << change_graph << novelty_graph << ecology_graph << complexity_graph;
  doc << lineage_viz;



  //viz.anim.Start();

}
