/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2025
*/
/**
 *  @file
 */

#include "emp/geometry/Line2D.hpp"
#include "emp/geometry/Polygon.hpp"
#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");
emp::Polygon poly(200, 300);
emp::Line2D line({5, 5}, {395, 395});
// emp::Palette::BLUE, emp::Palette::BLACK
emp::Circle circle{{150,150}, 50};
emp::Size2D can_size = {400, 400};
double poly_rot = 0.0;

void CanvasAnim(double time) {
  auto mycanvas = doc.Canvas("can");

  std::cerr << time << std::endl;

  // Update the circle position.
  circle += emp::Point{3, 0};
  if (circle.GetCenterX() >= can_size.Width() + circle.GetRadius()) circle.SetCenterX(-50);

  // Draw the new circle.
  mycanvas.Clear();
  mycanvas.Draw(circle, emp::Palette::BLUE, emp::Palette::BLACK);

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

// Local settings for Empecable file checker.
// empecable_words: poly
