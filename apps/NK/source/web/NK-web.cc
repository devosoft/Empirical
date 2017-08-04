//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

#include "../NKWorld.h"

namespace UI = emp::web;

UI::Document doc("emp_base");
NKWorld world;

int main()
{
  world.Setup();

  doc << "<h1>NK World</h1>";
  doc << "Update: " << UI::Live( [](){ return world.GetUpdate(); } ) << "<br>";
  doc << UI::Button(
    [](){
      world.RunStep();
      doc.Redraw();
    },
    "Step",
    "but_step"
  );
}
