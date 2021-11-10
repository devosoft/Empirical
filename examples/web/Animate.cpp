/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file Animate.cpp
 */

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  UI::CanvasPolygon poly;
  UI::CanvasLine line;

  emp::Random random;

  double cx = 150;
  double cy = 150;
  double cr = 50;
  double can_size = 400;
  double poly_rot = 0.0;

public:
  MyAnimate() : doc("emp_base"), poly( 200, 300, "red", "black"), line({5,5}, {395,395}, "green") {
    // How big should each canvas be?
    const double w = can_size;
    const double h = can_size;

    // Draw a simple circle animation on a canvas
    auto mycanvas = doc.AddCanvas(w, h, "can");
    mycanvas.Circle(cx, cy, cr, "blue", "purple");
    targets.push_back(mycanvas);

    // Draw the new polygon.
    poly.AddPoint(0,0).AddPoint(60,25).AddPoint(50,50).AddPoint(-50,50).AddPoint(25,40);
    mycanvas.Draw(poly);

    // Add a button.
    doc << "<br>";
    doc.AddButton([this](){
        ToggleActive();
        auto but = doc.Button("toggle");
        if (GetActive()) but.SetLabel("Pause");
        else but.SetLabel("Start");
      }, "Start", "toggle");

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;
  }

  void DoFrame() {
    auto mycanvas = doc.Canvas("can");

    // Update the circle position.
    cx+=3;
    if (cx >= can_size + cr) cx -= can_size;

    // Draw the new circle.
    mycanvas.Clear();
    mycanvas.Draw(emp::Circle(cx,cy,cr), "blue", "purple");
    if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "blue", "purple");

    // Update the polygon position
    poly_rot += 0.01;
    mycanvas.Rotate(poly_rot);
    mycanvas.Draw(poly);
    mycanvas.Rotate(-poly_rot);

    // Update the line.
    mycanvas.Draw(line);

    doc.Text("fps").Redraw();
  }
};

MyAnimate anim;

int main()
{
}
