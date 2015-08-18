#include "../../web/web.h"
#include "../../web/Tween.h"

namespace UI = emp::web;
UI::Document doc("emp_base");
UI::Tween tween(7);

double myvar = 20.0;

void SetVar(double v) { myvar = v; }

int main()
{
  UI::Text text("text");
  text.CSS("border", "3px solid blue").CSS("padding", "3px") << "Testing.  " << UI::Live(myvar);
  doc << text;

  tween.AddPath(SetVar, 0, 1000);
  tween.AddDependant(text);

  doc << UI::Button([](){tween.Start();}, "Start!");
}
