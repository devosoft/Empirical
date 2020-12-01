#include "../../UI/UI.h"

namespace UI = emp::UI;
UI::Document doc("emp_base");

int main() {

  UI::Initialize();
  
  doc << "<h1>Testing Canvas Object!</h1>"
      << UI::Image("motivator.jpg", "im").Size(200,200)
      << UI::Canvas(300,300,"cvs").StrokeColor("blue");

  auto & canvas = doc.Canvas("cvs");
  
  canvas.Rect(100,25,100,100, "red");
  canvas.Circle(50, 50, 45, "green", "purple");
  canvas.Rect(100,175,100,100, "yellow");
  
  doc.Update();
}

