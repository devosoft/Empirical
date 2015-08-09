#include "../../tools/alert.h"
#include "../../web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

std::string TestFun() { return "abcd"; }

int x = 20;
void IncVar() { x++; }

int main()
{
  UI::Text text("my_text");

  UI::Image motiv("../UI/motivator.jpg");



  text << "Testing testing!!!"
       << "<br>" << std::function<std::string()>(TestFun)
       << "<br>" << [](){ return emp::to_string(x); };

  doc << text << "<br>" << UI::Button(IncVar, "Test", "my_but") << "<br>" << motiv;

  UI::Button my_but = doc.FindButton("my_but");
  UI::Canvas canvas(200, 200);
  UI::Selector sel("sel");

  sel.SetOption("Option 1");
  sel.SetOption("Option B");
  sel.SetOption("Option the Forth");
  sel.SetOption("Option IV");

  doc << canvas << sel;
  canvas.Rect(0,0,200,200, "#AAAAAA", "black");
  canvas.Circle(100,75,50, "red", "black");

  my_but.AddDependent(text);
  my_but.Size(100,100);
  my_but.Background("blue");

  motiv.Size(100,100);
}
