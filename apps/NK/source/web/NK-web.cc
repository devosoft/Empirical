//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Developer Notes:
//  * Put update (And other stats) in a Text field.

#include "web/web.h"

#include "../NKWorld.h"

namespace UI = emp::web;

struct NKInterface {
  NKWorld world;

  UI::Document doc;
  UI::Div div_pop;
  UI::Div div_stats;
  UI::Canvas org_canvas;
  UI::Animate anim;

  NKInterface()
    : doc("emp_base")
    , div_pop("div_pop")
    , div_stats("div_stats")
    , org_canvas(400, 400, "org_canvas")
    , anim( [this](){ DoFrame(); }, org_canvas )
  {
    // Setup the NK World.
    world.Setup();

    // Setup the GUI Components.
    div_pop.SetCSS("float", "left");
    div_stats.SetCSS("float", "right");

    // Attach the GUI components to the web doc.
    doc << "<h1>NK World</h1>";
    doc << div_pop << div_stats;
    div_pop << org_canvas << "<br>";
    div_pop << UI::Button( [this](){ world.RunStep(); DrawAll(); }, "Step", "but_step" );
    div_pop << anim.GetToggleButton("but_toggle");

    div_stats << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } ) << "<br>";
  }

  void DrawOrgs() {
    org_canvas.Clear("black");
    for (size_t id = 0; id < world.GetSize() && id < 100; id++) {
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
