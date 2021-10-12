//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/geometry/Surface.hpp"
#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

struct Rock { size_t kills = 0; };
struct Paper { size_t kills = 0; };
struct Scissors { size_t kills = 0; };

UI::Document doc("emp_base");
double can_size = 400;
emp::Surface<Rock, Paper, Scissors> surface(can_size, can_size);


void CanvasAnim(double time) {
  auto mycanvas = doc.Canvas("can");

  std::cerr << time << std::endl;

  // Update the line.
  mycanvas.Draw(surface);

  doc.Text("fps").Redraw();
};

int main()
{
  // How big should each canvas be?
  const double w = can_size;
  const double h = can_size;

  emp::Random random;

  // Add objects
  size_t num_rocks = 10;
  size_t num_papers = 10;
  size_t num_scissors = 10;

  emp::vector<Rock> rocks(num_rocks);
  emp::vector<Paper> papers(num_papers);
  emp::vector<Scissors> scissors(num_scissors);

  for (size_t i = 0; i < num_rocks; i++) {
    surface.rocks[i]
  }

  // Draw a simple circle animation on a canvas
  auto mycanvas = doc.AddCanvas(w, h, "can");
  mycanvas.Circle(cx, cy, cr, "green", "purple");

  UI::Animate * anim = new UI::Animate(CanvasAnim, mycanvas);
  (void) anim;

  // Draw the new polygon.
  poly.AddPoint(0,0).AddPoint(60,25).AddPoint(50,50).AddPoint(-50,50).AddPoint(25,40);
  mycanvas.Draw(poly);

  doc << "<br>";
  doc.AddButton([anim](){
      anim->ToggleActive();
      auto but = doc.Button("toggle");
      if (anim->GetActive()) but.SetLabel("Pause");
      else but.SetLabel("Start");
    }, "Start", "toggle");

  doc << UI::Text("fps") << "FPS = " << UI::Live( [anim](){return anim->GetStepTime();} ) ;

}
