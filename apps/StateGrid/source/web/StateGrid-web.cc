//  This file is part of StateGrid
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "tools/Random.h"
#include "web/web.h"

#include "../SGWorld.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

constexpr size_t UPDATES = 10000;

int main()
{
  UI::Canvas canvas(400,400);

  doc << "<h1>Hello, world!</h1>";
  doc << canvas;


  emp::Random random;
  SGWorld world(random, "AvidaWorld");
  UI::Draw(canvas, (emp::StateGrid &) world.GetStateGrid(), emp::GetHueMap(5));


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    world.RunUpdate();
  }

  return 0;
}
