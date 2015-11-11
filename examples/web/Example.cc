// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int x = 20;

void IncX() { x++; doc.Redraw(); }

int main()
{
  doc << "<h1>This is my file!</h1>"
      << "And this is normal text" << "<p>"
      << "x = " << x << "<p>"
      << "(live) x = " << UI::Live(x) << "<p>";

  x = 100;
    
  doc << UI::Button(IncX, "Inc!");

  doc << UI::Image("motivator.jpg");
}
