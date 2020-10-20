//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/color_map.hpp"
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
  MyAnimate() : doc("emp_base"), poly(200, 300, "red", "black"), line({5,5}, {395,395}, "green") {
    // How big should each canvas be?
    const double w = can_size;
    const double h = can_size;

    // Draw a simple circle animation on a canvas
    auto mycanvas = doc.AddCanvas(w, h, "can");
    targets.push_back(mycanvas);

    // Add a button.
    doc << "<br>";
    doc.AddButton([this](){
        ToggleActive();
        auto but = doc.Button("toggle");
        if (GetActive()) but.SetLabel("Pause");
        else but.SetLabel("Start");
      }, "Start", "toggle");

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;

    // Draw some colors...
    auto color_map = emp::GetHSLMap(20, 400.0, 100.0, 100, 100, 20, 100);

    const double buffer = 20;
    const double radius = (can_size - 2 * buffer)/(color_map.size()*2);
    for (size_t i = 0; i < color_map.size(); i++) {
      int x_pos = (int) (buffer + (2*i+1) * radius);
      mycanvas.Circle(emp::Point(x_pos, 300), radius, color_map[i]);
      doc << "<br>" << color_map[i];
    }

  }

  void DoFrame() {
    auto mycanvas = doc.Canvas("can");

    // Update the circle position.
    cx+=3;
    if (cx >= can_size + cr) cx -= can_size;

    // Draw the new circle.
    mycanvas.Clear();
    mycanvas.Circle(cx, cy, cr, "blue", "purple");
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
