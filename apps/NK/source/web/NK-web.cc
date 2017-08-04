//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

#include "../NKWorld.h"

namespace UI = emp::web;

struct NKInterface {
  UI::Document doc;
  NKWorld world;

  UI::Canvas canvas;

  NKInterface()
    : doc("emp_base")
    , canvas(400, 400, "org_canvas")
  {
    world.Setup();
    doc << "<h1>NK World</h1>";
    doc << canvas << "<br>Update: " << UI::Live( [this](){ return world.GetUpdate(); } ) << "<br>";
    doc << UI::Button(
      [this](){ world.RunStep(); doc.Redraw(); },
      "Step",
      "but_step"
    );
  }
};

NKInterface interface;

int main()
{
}
