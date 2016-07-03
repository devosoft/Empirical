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

emp::web::GraphVisualization change_graph_1("Change", 400, 240);
emp::web::GraphVisualization novelty_graph_1("Novelty", 400, 240);
emp::web::GraphVisualization ecology_graph_1("Ecology", 400, 240);
emp::web::GraphVisualization complexity_graph_1("Complexity", 400, 240);

// emp::web::GraphVisualization change_graph_2("Change", 400, 250);
// emp::web::GraphVisualization novelty_graph_2("Novelty", 400, 250);
// emp::web::GraphVisualization ecology_graph_2("Ecology", 400, 250);
// emp::web::GraphVisualization complexity_graph_2("Complexity", 400, 250);
//
// emp::web::GraphVisualization change_graph_3("Change", 400, 250);
// emp::web::GraphVisualization novelty_graph_3("Novelty", 400, 250);
// emp::web::GraphVisualization ecology_graph_3("Ecology", 400, 250);
// emp::web::GraphVisualization complexity_graph_3("Complexity", 400, 250);

//emp::web::LineageVisualization lineage_viz(5000, 300);
NKAnimation<emp::BitVector, emp::evo::LineageStandard, emp::evo::OEEStats> simple_example_evo;
// NKAnimation<100, 0, emp::BitSet<100>, emp::evo::LineageStandard, emp::evo::OEEStats> pop_size_example_evo;
// int t = pop_size_example_evo.config.POP_SIZE(1000);
// NKAnimation<100, 20, emp::BitSet<100>, emp::evo::LineageStandard, emp::evo::OEEStats> k_example_evo;
// int g = k_example_evo.config.K(20);

emp::web::Document simple_example("simple_example");
// emp::web::Document pop_size_example("pop_size_example");
// emp::web::Document k_example("k_example");

int main()
{

  simple_example_evo.world->statsM.ConnectVis(&change_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&novelty_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&ecology_graph_1);
  simple_example_evo.world->statsM.ConnectVis(&complexity_graph_1);
  //
  // pop_size_example_evo.world->statsM.ConnectVis(&change_graph_2);
  // pop_size_example_evo.world->statsM.ConnectVis(&novelty_graph_2);
  // pop_size_example_evo.world->statsM.ConnectVis(&ecology_graph_2);
  // pop_size_example_evo.world->statsM.ConnectVis(&complexity_graph_2);
  //
  // k_example_evo.world->statsM.ConnectVis(&change_graph_3);
  // k_example_evo.world->statsM.ConnectVis(&novelty_graph_3);
  // k_example_evo.world->statsM.ConnectVis(&ecology_graph_3);
  // k_example_evo.world->statsM.ConnectVis(&complexity_graph_3);

  //viz.world->statsM.ConnectVis(&lineage_viz);
  //viz.world->lineageM.ConnectVis(&lineage_viz);

  //doc << "  <div class=\"reveal\"><div class=\"slides\">";
  //doc << "<br>";

  std::function<void(int)> set_n = [](int n){simple_example_evo.config.N(n);};
  emp::JSWrap(set_n, "set_n");

  std::function<void(int)> set_k = [](int k){
      if (k > simple_example_evo.config.N()){
        k = simple_example_evo.config.N();
      }
      simple_example_evo.config.K(k);
  };
  emp::JSWrap(set_k, "set_k");

  std::function<void(int)> set_pop_size = [](int pop_size){simple_example_evo.config.POP_SIZE(pop_size);};
  emp::JSWrap(set_pop_size, "set_pop_size");

  std::function<void(int)> set_tourn = [](int tourn){
      if (tourn > simple_example_evo.config.POP_SIZE()){
        tourn = simple_example_evo.config.POP_SIZE();
      }
      simple_example_evo.config.TOURNAMENT_SIZE(tourn);
  };
  emp::JSWrap(set_tourn, "set_tourn");

  std::function<void(double)> set_mut = [](double mut){simple_example_evo.config.MUT_RATE(mut);};
  emp::JSWrap(set_mut, "set_mut");

  std::function<void(bool)> set_sharing = [](bool sharing){simple_example_evo.config.FIT_SHARE(sharing);};
  emp::JSWrap(set_sharing, "set_sharing");

  std::function<void(bool)> set_change = [](bool change){simple_example_evo.config.CHANGE_ENV(change);};
  emp::JSWrap(set_change, "set_change");

  //simple_example << "<h2>Interactive Example</h1>";
  simple_example << "<h8>Fitness sharing: <input id=\"set_sharing\" name=\"set_sharing\" type=checkbox>";

  EM_ASM({d3.select("#set_sharing").on("change", function(){
    emp.set_sharing(this.checked);});
  });

  simple_example << "    Changing Environment: <input id=\"set_change\" name=\"set_change\" type=checkbox>";

  EM_ASM({d3.select("#set_change").on("change", function(){
      emp.set_change(this.checked);});
  });


  simple_example << "    N(<span id=\"n_disp\">50</span>): <input id=\"set_n\" name=\"set_n\" type=range min=1 max=500 value=50 onchange=\"emp.updateN(this.value)\">";

  EM_ASM({emp.updateN = function(val){
      document.getElementById("n_disp").innerHTML=val;};
  });

  EM_ASM({d3.select("#set_n").on("input", function(){
    emp.set_n(parseInt(this.value));})
  });

  simple_example << "K(<span id=\"k_disp\">0</span>): <input id=\"set_k\" name=\"set_k\" type=range min=0 max=20 value=0 onchange=\"emp.updateK(this.value)\">";

  EM_ASM({emp.updateK = function(val){
      document.getElementById("k_disp").innerHTML=val;};
  });

  EM_ASM({d3.select("#set_k").on("input", function(){
    emp.set_k(parseInt(this.value));})
  });

  simple_example << "Mutation Rate(<span id=\"mut_disp\">.0001</span>): <input id=\"set_mut\" name=\"set_mut\" type=range min=.00001 max=.1 step=.0001 value=.0001 onchange=\"emp.updateMut(this.value)\">";

  EM_ASM({emp.updateMut = function(val){
      document.getElementById("mut_disp").innerHTML=val;};
  });

  EM_ASM({d3.select("#set_mut").on("input", function(){
    emp.set_mut(parseFloat(this.value));})
  });

  simple_example << "<br>Pop Size(<span id=\"pop_disp\">100</span>): <input id=\"set_pop\" name=\"set_pop\" type=range min=5 max=1000 value=100 onchange=\"emp.updatePop(this.value)\">";

  EM_ASM({emp.updatePop = function(val){
      document.getElementById("pop_disp").innerHTML=val;};
  });

  EM_ASM({d3.select("#set_pop").on("input", function(){
    emp.set_pop_size(parseInt(this.value));})
  });

  simple_example << "Tournament Size(<span id=\"tourn_disp\">10</span>): <input id=\"set_tourn\" name=\"set_tourn\" type=range min=1 max=100 value=10 onchange=\"emp.updateTourn(this.value)\"></h8>";

  EM_ASM({emp.updateTourn = function(val){
      document.getElementById("tourn_disp").innerHTML=val;};
  });

  EM_ASM({d3.select("#set_tourn").on("input", function(){
    emp.set_tourn(parseInt(this.value));})
  });

  simple_example << change_graph_1 << novelty_graph_1 << ecology_graph_1 << complexity_graph_1;
  simple_example << "<br>";
  simple_example.AddButton([](){
      simple_example_evo.anim.ToggleActive();
      auto but = simple_example.Button("toggle");
      if (simple_example_evo.anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
  }, "Start", "toggle");

  simple_example.AddButton([](){
      change_graph_1.Clear();
      novelty_graph_1.Clear();
      ecology_graph_1.Clear();
      complexity_graph_1.Clear();
      simple_example_evo.NewWorld();
      simple_example_evo.world->statsM.ConnectVis(&change_graph_1);
      simple_example_evo.world->statsM.ConnectVis(&novelty_graph_1);
      simple_example_evo.world->statsM.ConnectVis(&ecology_graph_1);
      simple_example_evo.world->statsM.ConnectVis(&complexity_graph_1);
      //auto but = simple_example.Button("restart");
      //if (simple_example_evo.anim.GetActive()) but.Label("Pause");
      //else but.Label("Start");
  }, "Restart", "restart");


  //doc << lineage_viz;

  // pop_size_example << "<h1>What if we increase Pop Size?</h1>";
  // pop_size_example << "<h4>N: 50, K:0, Mutation rate: .0001, Pop size: 1000, Tournament Size: 10</h4>";
  // pop_size_example << change_graph_2 << novelty_graph_2 << ecology_graph_2 << complexity_graph_2;
  // pop_size_example << "<br>";
  // pop_size_example.AddButton([](){
  //     pop_size_example_evo.anim.ToggleActive();
  //     auto but = pop_size_example.Button("toggle2");
  //     if (pop_size_example_evo.anim.GetActive()) but.Label("Pause");
  //     else but.Label("Start");
  // }, "Start", "toggle2");
  //
  // k_example << "<h1>What if we increase K?</h1>";
  // k_example << "<h4>N: 50, K:10, Mutation rate: .0001, Pop size: 100, Tournament Size: 10</h4>";
  // k_example << change_graph_3 << novelty_graph_3 << ecology_graph_3 << complexity_graph_3;
  // k_example << "<br>";
  // k_example.AddButton([](){
  //     k_example_evo.anim.ToggleActive();
  //     auto but = k_example.Button("toggle3");
  //     if (k_example_evo.anim.GetActive()) but.Label("Pause");
  //     else but.Label("Start");
  // }, "Start", "toggle3");


  //viz.anim.Start();

}
