//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

#include "../config.h"
#include "../OpenWorld.h"

namespace UI = emp::web;

OpenWorldConfig config;
UI::Document doc("emp_base");
OpenWorld world(config);
UI::Animate anim;

void Animate(const UI::Animate & anim) {
  world.Update();

  UI::Draw( doc.Canvas("pop_view"),
            world.GetSurface(),
            emp::GetHueMap(360));
}

void DoStart() {
  anim.ToggleActive();
  auto start_but = doc.Button("start_but");
  auto step_but = doc.Button("step_but");

  if (anim.GetActive()) {
    start_but.SetLabel("Stop");    // If animation is running, button should read "Stop"
    step_but.SetDisabled(true);    // Cannot step animation already running.
  }
  else {
    start_but.SetLabel("Start");     // If animation is stopped, button should read "Start"
    step_but.SetDisabled(false);    // Can step stopped animation.
  }
}

void DoStep() {
  emp_assert(anim.GetActive() == false); // Step is only meaningful if the run is stopped.
  Animate(anim);
}

void DoReset() {
  world.Reset();

  // Redraw the world.
  UI::Draw( doc.Canvas("pop_view"),
            world.GetSurface(),
            emp::GetHueMap(360));
}

int emp_main()
{
  config.Read("OpenWorld.cfg", false);

  doc << "<h1>Hello, OpenWorld!</h1>";

  // Link keypresses to the proper handlers
  // keypress_manager.AddKeydownCallback(std::bind(&EvokeInterface::OnKeydown, this, _1));

  // Add a canvas to draw the world.
  doc << UI::Canvas(config.WORLD_X(), config.WORLD_Y(), "pop_view").SetPosition(10, 60) << "<br>";
  UI::Draw( doc.Canvas("pop_view"),
            world.GetSurface(),
            emp::GetHueMap(360));

  anim.SetCallback(Animate);

  // Add buttons.
  auto control_set = doc.AddDiv("buttons");
  control_set.SetPosition(10, 70+config.WORLD_Y());
  control_set << UI::Button(DoStart, "Start", "start_but");
  control_set << UI::Button(DoStep, "Step", "step_but");
  control_set << UI::Button(DoReset, "Reset", "reset_but");

  // Add Info
  auto data_set = doc.AddDiv("raw_data");
  data_set.SetPosition(70+config.WORLD_X(), 70);
  data_set << "Testing! " << UI::Live( [](){ return doc.Canvas("pop_view").GetYPos(); } ) << " XX";
}
