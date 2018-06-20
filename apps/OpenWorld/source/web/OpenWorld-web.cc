//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

#include "../config.h"
#include "../OpenWorld.h"

namespace UI = emp::web;

OpenWorldConfig config;
UI::Document doc("emp_base");
OpenWorld world(config);
  
int main()
{
  config.Read("OpenWorld.cfg");

  doc << "<h1>Hello, Open World!</h1>";

  // Link keypresses to the proper handlers
  // keypress_manager.AddKeydownCallback(std::bind(&EvokeInterface::OnKeydown, this, _1));

  // Add a canvas to draw the world.
  doc << UI::Canvas(config.WORLD_X(), config.WORLD_Y(), "pop_view").SetPosition(10, 60) << "<br>";
  UI::Draw( doc.Canvas("pop_view"),
            world.GetSurface(),
            emp::GetHueMap(360));

  // Add buttons.
  auto control_set = doc.AddDiv("buttons");
  control_set.SetPosition(10, 70+config.WORLD_Y());
  control_set << UI::Button([](){emp::Alert("Pressed Button!");}, "Start", "start_but");
  control_set << UI::Button([](){emp::Alert("Pressed Button!");}, "Step", "step_but");
  control_set << UI::Button([](){emp::Alert("Pressed Button!");}, "Reset", "reset_but");
}
