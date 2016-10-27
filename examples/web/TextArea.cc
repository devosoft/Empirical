//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE

#include "../../web/web.h"

namespace UI = emp::web;
UI::Document doc("emp_base");

UI::TextArea text_area([](const std::string &){ });

int main()
{
  doc << "<h1>Testing!</h1>";

  text_area.SetCallback([](const std::string & in){
      text_area.SetText("Changed!");
    });

  doc << text_area.SetSize(300,300);
}
