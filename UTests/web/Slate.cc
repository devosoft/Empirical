#include "../../web/web.h"
#include "../../emtools/emfunctions.h"

namespace UI = emp::web;
UI::Document doc("emp_base");

int myvar = 20;
int select_val = 0;

void IncVar() { myvar++; }

int main() {

  UI::Initialize();

  doc << "<h2>This is a second-level header!</h2>"
               << "<p>And here is some regular text."
               << "<p>Here is a variable: " << myvar;

  doc << "<br>Cur val = " << UI::Live(select_val);

  auto test_select = doc.AddSelector("test_select");
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
               << UI::Button([](){
                   auto & but = doc.Button("but");
                   but.Disabled(!but.IsDisabled());
                 }, "Disable Button");



  doc << "<p>" << UI::Slate("new_slate")
    .CSS("border", "5px solid red")
    .CSS("padding", "5px")
    .CSS("max-width", "580px")
    .CSS("border-radius", "15px")
               << "Testing out the new slate object with some wide text!";

  doc << "<br>" << UI::Image("motivator.jpg").Width(300).Opacity(0.8);

  myvar = 100;

  // doc.Button("but").Height(50).Background("green").CSS("border-radius", "5px");
  doc.Button("but").Height(50); // .Background("green").CSS("border-radius", "5px");

  // doc.Slate("new_slate")
  //   << "  And appending some more text onto the new slate.  Let's see how this all works out."
  //   << UI::Close("new_slate")
  //   << "And let's make sure this isn't in the red border.";

  std::stringstream os;
  doc << "<p>" << emp::text2html(os.str());
}

