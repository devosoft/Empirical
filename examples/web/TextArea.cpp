//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/web/web.hpp"

namespace UI = emp::web;
UI::Document doc("emp_base");

UI::TextArea text_area;
UI::TextArea text_area2;
UI::Text text_reflect;
UI::Text text_reflect2;
UI::Font out1_font;
UI::Font out2_font;

int main()
{
  doc << "<h1>Testing!</h1>";

  out1_font.SetColor("green");
  out2_font.SetColor("blue");
  text_reflect << out1_font;
  text_reflect2 << out2_font;

  text_area.SetCallback([](const std::string & in){
      text_area.SetText("Changed!");
    });

  text_area2.SetCallback([](const std::string & in){
      text_reflect.Clear();
      text_reflect << text_area2.GetText() << "<br>";
    });

  doc << text_area.SetSize(300,300)
      << text_area2.SetText("Starting text!").SetSize(300,200);

  doc << "<br>" << UI::Button([](){
                    text_reflect2.Clear();
                    text_reflect2 << text_area2.GetText() << "<br>";
                  }, "Update" )
      << "<br>" << text_reflect
      << "<br><br>" << text_reflect2;
}
