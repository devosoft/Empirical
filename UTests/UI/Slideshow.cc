#include "../../UI/UI.h"

#include "../../aps/Slideshow.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

emp::Slideshow show;

int main() {

  UI::Initialize();
  
  show << "<h1>Slide 1!!!</h1>";

  UI::document << "<h1>Testing Canvas Object!</h1>"
               << UI::Image("motivator.jpg", "im").Size(200,200)
               << UI::Canvas(300,300,"cvs").StrokeColor("blue");

  auto & canvas = UI::document.Canvas("cvs");
  
  canvas.Rect(100,25,100,100, "red");
  canvas.Circle(50, 50, 45, "green", "purple");
  canvas.Rect(100,175,100,100, "yellow");
  
  // UI::document.Update();
  show.Update();
}

// SLIDE 1
// Understanding Complexity Barriers in Evolving Systems
// By Emily Dolson, Anya Vostinar, Michael Wiser, and Charles Ofria
// BEACON Center for Evolution in Action

// SLIDE 2
// Introduction
// Informally open-endedness = “keep doing interesting things”
// Define “keep doing”
// Define “interesting things”
