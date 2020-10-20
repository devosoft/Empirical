//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/debug/alert.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

std::string TestFun() { return "abcd"; }

int x = 20;
void IncVar() { x++; }

int main()
{
  UI::Text text("my_text");

  UI::Image motiv("../UI/images/motivator.jpg");

  UI::Table tab(5,5);

  tab.SetCSS("border", "1px solid black");

  tab << "Test!";
  tab.GetCell(1,1) << "Test 2";

  text << "Testing testing!!!"
       << "<br>" << std::function<std::string()>(TestFun)
       << "<br>" << [](){ return emp::to_string(x); };

  doc << text << "<br>" << UI::Button(IncVar, "Test", "my_but") << "<br>" << motiv;

  UI::Button my_but = doc.Button("my_but");
  UI::Canvas canvas(200, 200);
  UI::Selector sel("sel");

  sel.SetOption("Option 1");
  sel.SetOption("Option B");
  sel.SetOption("Option the Third");
  sel.SetOption("Option IV");

  doc << canvas << sel << "<br>" << tab;
  canvas.Rect(0,0,200,200, "#AAAAAA", "black");
  canvas.Circle(100,75,50, "red", "black");

  my_but.AddDependant(text);
  my_but.SetSize(100,100);
  my_but.SetBackground("blue");

  motiv.SetSize(100,100);
}
