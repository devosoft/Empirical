//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "tools/Random.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"
#include "web/color_map.h"
#include "web/emfunctions.h"
#include "web/web.h"

namespace UI = emp::web;

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  UI::CanvasPolygon poly;
  UI::CanvasLine line;

  emp::Random random;

  double cx = 150.0;
  double cy = 150.0;
  double cr = 50;
  const double can_size = 700;

  emp::vector<emp::Point> position;
  emp::vector<emp::Point> velocity;
  const double image_size = 100.0;
  const size_t num_images = 100;

  const double can_limit = can_size - image_size;

public:
  MyAnimate() : doc("emp_base"), poly(200, 300, "red", "black"), line(5,5, 695, 695, "red") {
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
        if (GetActive()) but.Label("Pause");
        else but.Label("Start");
      }, "Start", "toggle");

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;

    // Draw some colors...
    auto color_map = emp::GetHSLMap(20, 400.0, 100.0, 100, 100, 20, 100);

    const double buffer = 20;
    const double radius = (can_size - 2 * buffer)/(color_map.size()*2);
    for (size_t i = 0; i < color_map.size(); i++) {
      double x_pos = (double) (buffer + (2*i+1) * radius);
      mycanvas.Circle(x_pos, 300.0, radius, color_map[i]);
      doc << "<br>" << color_map[i];
    }

    position.resize(num_images);
    velocity.resize(num_images);
    emp::Angle angle;

    for (size_t i = 0; i < num_images; i++) {
      position[i] = emp::Point(random.GetDouble(can_size), random.GetDouble(can_size));
      velocity[i] = angle.SetPortion(random.GetDouble()).GetPoint(random.GetDouble(1.0,3.0));
    }
  }

  void DoFrame() {
    auto mycanvas = doc.Canvas("can");

    // Update the circle position.
    cx+=3.0;
    if (cx >= can_size + cr) cx -= can_size;

    // Draw the new circle.
    mycanvas.Clear();
    mycanvas.Circle(cx, cy, cr, "blue", "purple");
    if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "blue", "purple");

    // Update the line.
    mycanvas.Draw(line);

    // Draw the cells...
    emp::RawImage cell("images/cell.png");
    emp::Point offsetX(can_size, 0.0);
    emp::Point offsetY(0.0, can_size);

    for (size_t i = 0; i < position.size(); i++) {
      mycanvas.Image(cell, position[i], image_size, image_size);
      if (position[i].GetX() > can_limit) mycanvas.Image(cell, position[i] - offsetX, image_size, image_size);
      if (position[i].GetY() > can_limit) mycanvas.Image(cell, position[i] - offsetY, image_size, image_size);
      position[i] += velocity[i];
      if (position[i].GetX() < 0.0) position[i] += offsetX;
      if (position[i].GetY() < 0.0) position[i] += offsetY;
      if (position[i].GetX() > can_size) position[i] -= offsetX;
      if (position[i].GetY() > can_size) position[i] -= offsetY;
    }

    doc.Text("fps").Redraw();


  }
};

MyAnimate anim;

int main()
{
}
