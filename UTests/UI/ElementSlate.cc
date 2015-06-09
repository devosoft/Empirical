#include "../../UI/UI.h"

namespace UI = emp::UI;

int myvar = 20;

void IncVar() { myvar++; UI::document.Update(); }

int main() {

  UI::Initialize();

  UI::document << "<h2>This is a header!</h2>"
               << "<p>And here is some regular text."
               << "<p>Here is a variable: " << myvar;

  UI::document << UI::Text("my_text").Background("#DDDDFF").CSS("color", "#550055")
               << "Is this text formatted?";

  UI::document << UI::Text() << "<p>Here is an updating variable: " << UI::Var(myvar)
               << "<br>" << UI::Button(IncVar, "MyButton", "but");


  UI::document << UI::Slate("new_slate").CSS("width", 100) << "Testing out the new slate object with some wide text!";

  UI::document << "<br>" << UI::Image("motivator.jpg").Width(600).Opacity(0.8);

  myvar = 100;

  UI::document.Button("but").Height(50).Background("green");

  UI::document.Update();
}

