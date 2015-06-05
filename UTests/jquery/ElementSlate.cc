#include "../../UI/UI.h"

namespace UI = emp::UI;

int myvar = 20;

void IncVar() { myvar++; UI::document.Update(); }

int main() {

  UI::Initialize();

  UI::document << "<h2>This is a header!</h2>"
               << "<p>And here is some regular text."
               << "<p>Here is a variable: " << myvar;

  UI::document << "<p>Here is an updating variable: " << UI::Var(myvar)
               << "<br>" << UI::Button(IncVar, "MyButton", "but");

  UI::document << "<br>" << UI::Image("motivator.jpg").Width(600);

  myvar = 100;

  UI::document.Button("but").Height(50);

  UI::document.Update();
}

