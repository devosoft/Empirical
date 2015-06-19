#include "../../UI/UI.h"
#include "../../emtools/emfunctions.h"

namespace UI = emp::UI;

int myvar = 20;

void IncVar() { myvar++; UI::document.Update(); }

UI::ElementSlate doc2(UI::Slate("emp_base"));

int main() {

  UI::Initialize();

  doc2 << "This is the alternate page!"
       << UI::Button( [](){ UI::document.Update(); }, "Swap Back!" );

  UI::document << "<h2>This is a second-level header!</h2>"
               << "<p>And here is some regular text."
               << "<p>Here is a variable: " << myvar;

  UI::document << UI::Button( [](){ doc2.Update(); }, "Swap State");

  UI::document << UI::Text("my_text").Background("#DDDDFF")
    .CSS("color", "#550055")
    .CSS("border", "3px solid blue")
    .CSS("padding", "3px")
    .CSS("border-radius", "5px")
    //    .CSS("position", "fixed")
               << "Is this text formatted?";

  UI::document << UI::Text() << "<p>Here is an updating variable: " << UI::Var(myvar)
               << "<br>"
               << UI::Button(IncVar, "MyButton", "but")
               << UI::Button([](){ auto & but = UI::document.Button("but");
                   but.Disabled(!but.IsDisabled()); but.Update(); }, "Disable Button");



  UI::document << "<p>" << UI::Slate("new_slate")
    .CSS("border", "5px solid red")
    .CSS("padding", "5px")
    .CSS("max-width", "580px")
    .CSS("border-radius", "15px")
               << "Testing out the new slate object with some wide text!";

  UI::document << "<br>" << UI::Image("motivator.jpg").Width(600).Opacity(0.8);

  myvar = 100;

  UI::document.Button("but").Height(50).Background("green").CSS("border-radius", "5px");

  UI::document.Slate("new_slate")
    << "  And appending some more text onto the new slate.  Let's see how this all works out."
    << UI::Close("new_slate")
    << "And let's make sure this isn't in the red border.";

  std::stringstream os;
  UI::document.OK(os, true);
  UI::document << "<p>" << emp::text2html(os.str());

  UI::document.Update();
}

