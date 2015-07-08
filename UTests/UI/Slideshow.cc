#include "../../UI/UI.h"

#include "../../aps/Slideshow.h"
#include "../../emtools/emfunctions.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

emp::Slideshow show;

void NextSlide() { show.NextSlide(); }
void PrevSlide() { show.PrevSlide(); }

int main() {

  UI::Initialize();
  emp::SetBackgroundColor("black");
  emp::SetColor("cyan");
  
  show.ActivateKeypress();

  show << UI::Text("title") << "Understanding Complexity Barriers in Evolving Systems<br><br>"
       << UI::Text("authors") << "By Emily Dolson, Anya Vostinar, Michael Wiser, and Charles Ofria<br><br>BEACON Center for the Study of Evolution in Action<br>";
  show.GetSlide().CSS("text-align", "center");
  show.GetSlide().Text("title").CSS("font-size", "50px");
  show.GetSlide().Text("authors").CSS("font-size", "25px").Color("white").CSS("width", "70%").Center();


  show.NewSlide("Introduction");

  show << "<h1>Testing Canvas Object!</h1>"
       << UI::Image("motivator.jpg", "im").Size(200,200)
       << UI::Canvas(300,300,"cvs").StrokeColor("blue");
  auto & canvas = show.GetSlide().Canvas("cvs");
  canvas.Rect(100,25,100,100, "red");
  canvas.Circle(50, 50, 45, "green", "purple");
  canvas.Rect(100,175,100,100, "yellow");


  show.NewSlide("How do we define \"keep going\"?");

  show.NewSlide("Change Potential");

  show.NewSlide("Novelity Potential");

  show.NewSlide("Complexity Potential");

  show.NewSlide("Ecological Potential");

  show.NewSlide("Measurement Techniques");

  show.NewSlide("Results");

  show.NewSlide("Acknowledgements");


  // UI::document.Update();
  show.Start();
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
