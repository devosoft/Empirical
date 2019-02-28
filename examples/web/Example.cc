//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int x;

int emp_main()
{
  doc << "<h1>Button test!</h1>";

  x = 10;

  doc << "x = " << UI::Live(x) << "<br>";

  x = 20;

  doc << UI::Button([](){x++; doc.Redraw();}, "Inc", "inc_but");

  doc.Redraw();
}
