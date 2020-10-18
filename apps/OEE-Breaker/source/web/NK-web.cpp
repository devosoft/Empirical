//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Organisms are bitstrings; fitness is based on the number of times a pattern
//  is repeated.  Combined with ecological pressures (to promote differentiation)
//  this program should pass most open-ended evolution tests.  Instictively,
//  however, these sorts of bitstrings don't seem actually open-ended.

#include "web/web.h"

#include "../OEEB-World.h"

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
    div_pop.SetSize(400,400).SetScrollAuto(); //.SetResizable();

    // Attach the GUI components to the web doc.
    div_controls << UI::Button( [this](){ world.RunStep(); DrawAll(); }, "Step", "but_step" );
    div_controls << anim.GetToggleButton("but_toggle");
    div_controls << UI::Button( [this](){ world.Reset(); world.Setup(); DrawAll(); }, "Reset", "but_reset");
    // div_controls << UI::Button( [this](){
    //     emp::Alert("x=", div_pop.GetXPos(), " y=", div_pop.GetYPos(),
    //                " width=", div_pop.GetWidth(), " height=", div_pop.GetHeight());
    //   }, "Alert");
    // div_pop << "<br>";
    div_pop << org_canvas;

    auto & fit_node = world.GetFitnessDataNode();
    div_stats << "<b>Stats:</b>";
    div_stats << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } );
    div_stats << "<br>Min Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMin(); } );
    div_stats << "<br>Mean Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMean(); } );
    div_stats << "<br>Max Fitness: " << UI::Live( [&fit_node](){ return fit_node.GetMax(); } );

    doc << "<h1>NK World</h1>";
    doc << div_pop;
    doc << div_stats;
    doc << div_controls;
    doc << div_vis;

    // Place divs in reasonable positions.

    emp::web::OnDocumentReady( [this](){ LayoutDivs(); DrawAll(); } );
  }

  ///  Restore the proper layout of divs, even once portions change size.
  ///
  ///   x1       x2
  ///   +--------+-----------+ y1
  ///   |        |  CONTROLS |
  ///   +   POP  +-----------+ y2
  ///   |        |   STATS   |
  ///   +--------+-----------+ y3
  ///   |                    |
  ///   |   VISUALIZATIONS   |
  ///   |                    |
  ///   +--------------------+

  void LayoutDivs() {
    const double spacing = 10;
    const double x1 = div_pop.GetXPos();
    const double x2 = x1 + div_pop.GetOuterWidth() + spacing;
    const double y1 = div_pop.GetYPos();
    const double y2 = y1 + div_controls.GetOuterHeight() + spacing;
    const double y3a = y1 + div_pop.GetOuterHeight();
    const double y3b = y2 + div_stats.GetOuterHeight();
    const double y3 = emp::Max(y3a, y3b) + spacing;
    div_controls.SetPosition(x2, y1);
    div_stats.SetPosition(x2, y2);
    div_vis.SetPosition(x1,y3);
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

    LayoutDivs();
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
