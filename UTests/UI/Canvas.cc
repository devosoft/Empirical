#include "../../UI/UI.h"

namespace UI = emp::UI;

int main() {

  UI::Initialize();
  
  UI::document << "<h1>Testing Canvas Object!</h1>"
               << UI::Image("motivator.jpg", "im").Size(200,200)
               << UI::Canvas(300,300,"cvs").StrokeColor("blue");

  auto & canvas = UI::document.Canvas("cvs");
  
  canvas.Rect(100,25,100,100, "red");
  canvas.Circle(50, 50, 45, "green", "purple");
  canvas.Rect(100,175,100,100, "yellow");
  
  UI::document.Update();
}

