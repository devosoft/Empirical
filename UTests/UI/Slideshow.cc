#include "../../UI/UI.h"

#include "../../aps/Slideshow.h"
#include "../../emtools/emfunctions.h"

namespace UI = emp::UI;

std::unique_ptr<emp::Slideshow> slideshow;

int main() {

  UI::Initialize();
  emp::SetBackgroundColor("gray");
  emp::SetColor("cyan");

	// Create slideshow
	slideshow.reset(new emp::Slideshow("slideshow_base", emp::defaults::TITLE_HEIGHT));
	emp::Slideshow &show = *slideshow;
  show.ActivateKeypress();

	// Title Slide
  int title_height = 50;
	show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Understanding Complexity Barriers in Evolving Systems";
  show << UI::Text("authors").FontSize(title_height).Color("black").CSS("width", "70%").Center()
       << "By Emily Dolson, Anya Vostinar, Michael Wiser, and Charles Ofria<br><br>BEACON Center for the Study of Evolution in Action<br>";

	// Introduction Slide
  show.NewSlide();
	show << UI::Text("title").FontSizeVW(title_height).Center() << "Introduction";
	//show.Text("title").PreventAppend()
  show << "<h1>Testing Canvas Object!</h1>"
       << UI::Image("motivator.jpg", "im").Size(200,200)
       << UI::Canvas(300,300,"cvs").StrokeColor("blue");
  auto & canvas = show.GetSlide().Canvas("cvs");
  canvas.Rect(100,25,100,100, "red");
  canvas.Circle(50, 50, 45, "green", "purple");
  canvas.Rect(100,175,100,100, "yellow");


  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "How do we define \"keep going\"?";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Change Potential";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Novelity Potential";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Complexity Potential";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Ecological Potential";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Measurement Techniques";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Results";

  show.NewSlide();
	show << UI::Text("title").Color("black").FontSize(title_height*1.3).Center()
			 << "Acknowledgements";

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
