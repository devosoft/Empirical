/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ElementSlate.cpp
 */

#include "../../UI/UI.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;
UI::Document doc("emp_base");

int myvar = 20;
int select_val = 0;

void IncVar() { myvar++; doc.Update(); }

UI::ElementSlate doc2(UI::Slate("emp_base"));

int main() {

  UI::Initialize();

  doc2 << "This is the alternate page!"
       << UI::Button( [](){ doc.Update(); }, "Swap Back!" );

  doc << "<h2>This is a second-level header!</h2>"
               << "<p>And here is some regular text."
               << "<p>Here is a variable: " << myvar;

  doc << UI::Button( [](){ doc2.Update(); }, "Swap State");

  doc << "<br>Cur val = " << UI::Live(select_val);
  // doc << UI::Selector( std::function<void(int)>( [](int x){ select_val=x; doc.Update(); } ),
  //                      emp::vector<std::string>({"a", "b", "c"}) );

  auto & test_select = doc.AddSelector("test_select");
  test_select.SetOption("Option 1");
  test_select.SetOption("Option B");
  test_select.SetOption("Option Three");
  test_select.SetOption("Option IV");
  // emp::Alert("Select size = ", test_select.GetNumOptions());


  doc << UI::Text("my_text").Background("#DDDDFF")
    .CSS("color", "#550055")
    .CSS("border", "3px solid blue")
    .CSS("padding", "3px")
    .CSS("border-radius", "5px")
    //    .CSS("position", "fixed")
               << "Is this text formatted?";

  doc << UI::Text() << "<p>Here is an updating variable: " << UI::Live(myvar)
               << "<br>"
               << UI::Button(IncVar, "MyButton", "but")
               << UI::Button([](){ auto & but = doc.Button("but");
                   but.Disabled(!but.IsDisabled()); but.Update(); }, "Disable Button");



  doc << "<p>" << UI::Slate("new_slate")
    .CSS("border", "5px solid red")
    .CSS("padding", "5px")
    .CSS("max-width", "580px")
    .CSS("border-radius", "15px")
               << "Testing out the new slate object with some wide text!";

  doc << "<br>" << UI::Image("motivator.jpg").Width(600).Opacity(0.8);

  myvar = 100;

  doc.Button("but").Height(50).Background("green").CSS("border-radius", "5px");

  doc.Slate("new_slate")
    << "  And appending some more text onto the new slate.  Let's see how this all works out."
    << UI::Close("new_slate")
    << "And let's make sure this isn't in the red border.";

  std::stringstream os;
  doc.OK(os, true);
  doc << "<p>" << emp::text2html(os.str());

  doc.Update();
}
