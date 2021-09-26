/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015
 *
 *  @file Slideshow.cpp
 */

#include "../../UI/UI.h"

#include "../../aps/Slideshow.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

emp::Slideshow show("Understanding Complexity Barriers in Evolving Systems");

int main() {

  UI::Initialize();
  emp::SetBackgroundColor("gray");
  emp::SetColor("cyan");

  show.ActivateKeypress();

  show << UI::Text("authors") << "By Emily Dolson, Anya Vostinar, Michael Wiser, and Charles Ofria<br><br>BEACON Center for the Study of Evolution in Action<br>";
  show.GetSlide().CSS("text-align", "center");
  show.GetSlide().Text("authors").Color("white").CSS("width", "70%").Center();
  show << UI::Image("Devolab.png").WidthVW(30)
       << UI::Image("BEACON.png").WidthVW(18);

  show.NewSlide("Introduction");
  show << UI::Text("intro").SetPositionVW(25, 20).Color("white")
       << "<big>Informally open-endedness =<br>"
       << "\"Keep Doing Interesting Things\"</big>";

  show.NewSlide("Introduction");
  show << UI::Text("intro").SetPositionVW(25, 20).Color("white")
       << "<big>Informally open-endedness =<br>"
       << "\"<b><font color=\"red\">Keep Doing</font></b> Interesting Things\"</big>";


  // show << "<h1>Testing Canvas Object!</h1>"
  //      << UI::Image("motivator.jpg", "im").Size(200,200)
  //      << UI::Canvas(300,300,"cvs").StrokeColor("blue");
  // auto & canvas = show.GetSlide().Canvas("cvs");
  // canvas.Rect(100,25,100,100, "red");
  // canvas.Circle(50, 50, 45, "green", "purple");
  // canvas.Rect(100,175,100,100, "yellow");


  show.NewSlide("How do we define \"Keep Doing\"?");

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
