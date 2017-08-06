//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

#include "../NKWorld.h"

namespace UI = emp::web;

struct NKInterface {
  UI::Document doc;
  NKWorld world;

  UI::Canvas org_canvas;
  UI::Animate anim;

  NKInterface()
    : doc("emp_base")
    , org_canvas(400, 400, "org_canvas")
    , anim( [this](){ DoFrame(); }, org_canvas )
  {
    world.Setup();
    doc << "<h1>NK World</h1>";
    doc << org_canvas << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } ) << "<br>";
    doc << UI::Button(
      [this](){ world.RunStep(); Redraw(); },
      "Step",
      "but_step"
    );
    doc.AddButton([this](){
        anim.ToggleActive();
        auto but = doc.Button("but_toggle");
        if (anim.GetActive()) but.Label("Pause");
        else but.Label("Start");
      }, "Start", "but_toggle");
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

  void Redraw() {
    doc.Freeze();
    DrawOrgs();
    doc.Activate();
  }

  void DoFrame() {
    world.RunStep();
    Redraw();
  }
};

NKInterface interface;

int main()
{
}
