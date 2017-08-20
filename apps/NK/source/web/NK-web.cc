//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file contains an easy-to-manipulate NK world.

#include "web/web.h"

#include "../NKWorld.h"

namespace UI = emp::web;

struct NKInterface {
  NKWorld world;

  UI::Document doc;
  UI::Div div_pop;
  UI::Div div_stats;
  UI::Div div_controls;
  UI::Div div_vis;

  UI::Canvas pop_canvas;
  UI::Canvas org_canvas;
  UI::Animate anim;

  NKInterface()
    : doc("emp_base")
    , div_pop("div_pop"), div_stats("div_stats"), div_controls("div_controls"), div_vis("div_vis")
    , pop_canvas(400, 400, "pop_canvas"), org_canvas(800, 800, "org_canvas")
    , anim( [this](){ DoFrame(); }, org_canvas )
  {
    // Setup the NK World.
    world.Setup();

    // Setup initial sizes for divs.
    div_pop.SetSize(400,400); // .SetScrollAuto().SetResizable();

    // Attach the GUI components to the web doc.
    div_pop << UI::Button( [this](){ world.RunStep(); DrawAll(); }, "Step", "but_step" );
    div_pop << anim.GetToggleButton("but_toggle");
    div_pop << UI::Button( [this](){
        emp::Alert("x=", div_pop.GetXPos(), " y=", div_pop.GetYPos(),
                   " width=", div_pop.GetWidth(), " height=", div_pop.GetHeight());
      }, "Alert");
    div_pop << "<br>";
    div_pop << org_canvas;

    auto & fit_node = world.GetFitnessDataNode();
    div_stats << "<b>Stats:</b>";
    div_stats << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } );
    div_stats << "<br>Min Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMin(); } );
    div_stats << "<br>Mean Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMean(); } );
    div_stats << "<br>Max Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMax(); } );

    doc << "<h1>NK World</h1>";
    doc << div_pop << div_stats;

    // Place divs in reasonable positions.

    LayoutDivs();
  }

  void LayoutDivs() {
    div_stats.SetPosition(450, 30);
  }

  void DrawOrgs() {
    // double width = world.N * 4;
    // double height = world.GetSize();
    org_canvas.SetSize(4*world.N, 4 * world.GetSize());
    org_canvas.Clear("black");
    for (size_t id = 0; id < world.GetSize(); id++) {
      auto & org = world[id];
      for (size_t pos = 0; pos < org.GetSize(); pos++) {
        if (!org[pos]) continue;
        org_canvas.Rect(pos*4, id*4, 4, 4, "yellow");
      }
    }
  }

  void DrawAll() {
    DrawOrgs();
    div_stats.Redraw();
  }

  void DoFrame() {
    world.RunStep();
    DrawAll();
  }
};

NKInterface interface;

int main()
{
}
