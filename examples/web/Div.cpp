/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file Div.cpp
 */

#include "emp/web/commands.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
UI::Document doc("emp_base");

int myvar = 20;
int select_val = 0;

void IncVar() { myvar++; }

int main() {

  doc << "<h2>This is a second-level header!</h2>"
      << "<p>And here is some regular text."
      << "<p>Here is a variable: " << myvar;

  doc << "<br>Cur val = " << UI::Live(select_val);

  doc << UI::endl; // PrintStr("abc");
  doc << UI::endl; // PrintStr("abc");
  doc << UI::PrintStr("abc");
  doc << UI::endl; // PrintStr("abc");
  doc << UI::endl; // PrintStr("abc");

  auto test_select = doc.AddSelector("test_select");
  test_select.SetOption("Option 1");
  test_select.SetOption("Option B");
  test_select.SetOption("Option Three");
  test_select.SetOption("Option IV");
  // emp::Alert("Select size = ", test_select.GetNumOptions());


  doc << UI::Text("my_text").SetBackground("#DDDDFF")
    .SetCSS("color", "#550055")
    .SetCSS("border", "3px solid blue")
    .SetCSS("padding", "3px")
    .SetCSS("border-radius", "5px")
    //    .SetCSS("position", "fixed")
      << "Is this text formatted?";

  doc << UI::Text("ud_text") << "<p>Here is an updating variable: " << UI::Live(myvar)
               << "<br>"
               << UI::Button(IncVar, "MyButton", "but")
               << UI::Button([](){
                   auto but = doc.Button("but");
                   but.SetDisabled(!but.IsDisabled());
                 }, "Disable Button");

  doc.Button("but").SetTitle("Testing if button titles do proper hover-over!");


  doc << "<p>" << UI::Div("new_slate")
    .SetCSS("border", "5px solid red")
    .SetCSS("padding", "5px")
    .SetCSS("max-width", "200px")
    .SetCSS("border-radius", "15px")
      << "Testing out the new slate object with some wide text!";

  doc << "<br>" << UI::Image("images/motivator.jpg").SetWidth(300).SetOpacity(0.8);

  myvar = 100;

  doc.Button("but").SetHeight(50).SetBackground("green").SetCSS("border-radius", "5px")
    .AddDependant(doc.Text("ud_text"));


  doc.Div("new_slate")
    << "  And appending some more text onto the new slate.  Let's see how this all works out."
    << UI::Close("new_slate")
    << "And let's make sure this isn't in the red border.";

  std::stringstream os;
  doc << "<p>" << emp::text2html(os.str());
}
