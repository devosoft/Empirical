//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"

namespace UI = emp::web;
UI::Document doc("emp_base");

UI::TextArea text_area([](const std::string &){ });
UI::TextArea text_area2([](const std::string &){ });
UI::Text text_reflect;

int main()
{
  doc << "<h1>Testing!</h1>";

  text_area.SetCallback([](const std::string & in){
      text_area.SetText("Changed!");
    });

  doc << text_area.SetSize(300,300)
      << text_area2.SetText("Starting text!").SetSize(300,200);

  doc << "<br>" << UI::Button([](){ text_reflect.Clear(); text_reflect << text_area2.GetText(); }, "Update" )
      << "<br>" << text_reflect;
}
