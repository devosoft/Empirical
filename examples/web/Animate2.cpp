//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");
UI::CanvasPolygon poly(200, 300, "red", "black");
UI::CanvasLine line(5,5, 395,395, "green");

double cx = 150;
double cy = 150;
double cr = 50;
double can_size = 400;
double poly_rot = 0.0;

void CanvasAnim(double time) {
  auto mycanvas = doc.Canvas("can");

  std::cerr << time << std::endl;

  // Update the circle position.
  cx+=3;
  if (cx >= can_size + cr) cx -= can_size;

  // Draw the new circle.
  mycanvas.Clear();
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "green", "purple");

  // Update the polygon position
  poly_rot += 0.01;
  mycanvas.Rotate(poly_rot);
  mycanvas.Draw(poly);
  mycanvas.Rotate(-poly_rot);

  // Update the line.
  mycanvas.Draw(line);

  doc.Text("fps").Redraw();
};

int main()
{
  // How big should each canvas be?
  const double w = can_size;
  const double h = can_size;

  emp::Random random;

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
