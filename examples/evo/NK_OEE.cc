//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file explores the template defined in evo::Population.h


#include "../../web/web_init.h"
#include "../../tools/BitSet.h"
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
#include "../../tools/Random.h"
#include "../../evo/StatsManager.h"

emp::web::GraphVisualization change_graph_1("Change", 400, 250);
emp::web::GraphVisualization novelty_graph_1("Novelty", 400, 250);
emp::web::GraphVisualization ecology_graph_1("Ecology", 400, 250);
emp::web::GraphVisualization complexity_graph_1("Complexity", 400, 250);

emp::web::GraphVisualization change_graph_2("Change", 400, 250);
emp::web::GraphVisualization novelty_graph_2("Novelty", 400, 250);
emp::web::GraphVisualization ecology_graph_2("Ecology", 400, 250);
emp::web::GraphVisualization complexity_graph_2("Complexity", 400, 250);

emp::web::GraphVisualization change_graph_3("Change", 400, 250);
emp::web::GraphVisualization novelty_graph_3("Novelty", 400, 250);
emp::web::GraphVisualization ecology_graph_3("Ecology", 400, 250);
emp::web::GraphVisualization complexity_graph_3("Complexity", 400, 250);

//emp::web::LineageVisualization lineage_viz(5000, 300);
NKAnimation<100, 0, emp::BitSet<100>, emp::evo::LineageStandard, emp::evo::OEEStats> simple_example_evo;
NKAnimation<100, 0, emp::BitSet<100>, emp::evo::LineageStandard, emp::evo::OEEStats> pop_size_example_evo;
int t = pop_size_example_evo.config.POP_SIZE(1000);
NKAnimation<100, 20, emp::BitSet<100>, emp::evo::LineageStandard, emp::evo::OEEStats> k_example_evo;
int g = k_example_evo.config.K(20);

emp::web::Document simple_example("simple_example");
emp::web::Document pop_size_example("pop_size_example");
emp::web::Document k_example("k_example");

int main()
{

  simple_example_evo.world->statsM.ConnectVis(&change_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&novelty_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&ecology_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&complexity_graph_1);

  pop_size_example_evo.world->statsM.ConnectVis(&change_graph_2);
  pop_size_example_evo.world->statsM.ConnectVis(&novelty_graph_2);
  pop_size_example_evo.world->statsM.ConnectVis(&ecology_graph_2);
  pop_size_example_evo.world->statsM.ConnectVis(&complexity_graph_2);

  k_example_evo.world->statsM.ConnectVis(&change_graph_3);
  k_example_evo.world->statsM.ConnectVis(&novelty_graph_3);
  k_example_evo.world->statsM.ConnectVis(&ecology_graph_3);
  k_example_evo.world->statsM.ConnectVis(&complexity_graph_3);

  //viz.world->statsM.ConnectVis(&lineage_viz);
  //viz.world->lineageM.ConnectVis(&lineage_viz);

  //doc << "  <div class=\"reveal\"><div class=\"slides\">";
  //doc << "<br>";
  simple_example << "<h1>A Simple Example</h1>";
  simple_example << "<h4>N: 50, K:0, Mutation rate: .0001, Pop size: 100, Tournament Size: 10</h4>";
  simple_example << change_graph_1 << novelty_graph_1 << ecology_graph_1 << complexity_graph_1;
  simple_example << "<br>";
  simple_example.AddButton([](){
      simple_example_evo.anim.ToggleActive();
      auto but = simple_example.Button("toggle");
      if (simple_example_evo.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Start", "toggle");
  //doc << lineage_viz;

  pop_size_example << "<h1>What if we increase Pop Size?</h1>";
  pop_size_example << "<h4>N: 50, K:0, Mutation rate: .0001, Pop size: 1000, Tournament Size: 10</h4>";
  pop_size_example << change_graph_2 << novelty_graph_2 << ecology_graph_2 << complexity_graph_2;
  pop_size_example << "<br>";
  pop_size_example.AddButton([](){
      pop_size_example_evo.anim.ToggleActive();
      auto but = pop_size_example.Button("toggle2");
      if (pop_size_example_evo.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Start", "toggle2");

  k_example << "<h1>What if we increase K?</h1>";
  k_example << "<h4>N: 50, K:10, Mutation rate: .0001, Pop size: 100, Tournament Size: 10</h4>";
  k_example << change_graph_3 << novelty_graph_3 << ecology_graph_3 << complexity_graph_3;
  k_example << "<br>";
  k_example.AddButton([](){
      k_example_evo.anim.ToggleActive();
      auto but = k_example.Button("toggle3");
      if (k_example_evo.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Start", "toggle3");


  //viz.anim.Start();

}
