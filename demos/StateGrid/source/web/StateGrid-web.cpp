//  This file is part of StateGrid
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "tools/Random.h"
#include "web/web.h"

#include "../SGWorld.h"

namespace UI = emp::web;

constexpr size_t UPDATES = 10000;

struct SGInterface {
  SGWorld world;
  emp::Random random;
  size_t update;

  UI::Document doc;
  UI::Canvas canvas;

  UI::Animate anim;

  SGInterface()
    : world(random, "AvidaWorld")
    , update(0)
    , doc("emp_base")
    , canvas(400,400)
    , anim( [this](){ DoFrame(); }, canvas )
  {
    doc << "<h1>Hello, world!</h1>";
    doc << "Update = " << UI::Live(update) << "<br>";
    doc << canvas;
    UI::Draw(canvas, (emp::StateGrid &) world.GetStateGrid(), emp::GetHueMap(5));
  }

  void DrawAll() {
    //DrawOrgs();
    doc.Redraw();

    //LayoutDivs();
  }

  void DoFrame() {
    world.RunUpdate();
    DrawAll();
  }
};


SGInterface interface;

int main()
{
  return 0;
}
