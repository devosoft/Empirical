//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/math/Random.hpp"
#include "emp/web/Animate.hpp"
#include "emp/web/canvas_utils.hpp"
#include "emp/web/color_map.hpp"
#include "emp/web/emfunctions.hpp"
#include "emp/web/Input.hpp"
#include "emp/web/web.hpp"

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

  double test_var = 0.5;

  emp::vector<emp::Point> position;
  emp::vector<emp::Point> velocity;
  double image_size = 100.0;
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
        if (GetActive()) but.SetLabel("Pause");
        else but.SetLabel("Start");
      }, "Start", "toggle");

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;

    // Draw some colors...
    auto color_map = emp::GetHSLMap(20, 400.0, 100.0, 100, 100, 20, 100);

    position.resize(num_images);
    velocity.resize(num_images);
    emp::Angle angle;

    for (size_t i = 0; i < num_images; i++) {
      position[i] = emp::Point(random.GetDouble(can_size), random.GetDouble(can_size));
      velocity[i] = angle.SetPortion(random.GetDouble()).GetPoint(random.GetDouble(1.0,3.0));
    }

    UI::Div slider_info_div;
    UI::Div slider_div;

    slider_info_div << UI::Live(test_var) << " ";
    UI::Input cell_count_slider([this, slider_info_div](std::string val) mutable { test_var = emp::from_string<double>(val); slider_info_div.Redraw(); }, "range", "cell_count");
    slider_div << cell_count_slider;

    doc << slider_info_div;
    doc << slider_div;

    DoFrame();
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
    emp::Point offsetXY(can_size, can_size);

    for (size_t i = 0; i < position.size(); i++) {
      mycanvas.Image(cell, position[i], image_size, image_size);
      const bool x_wrap = (position[i].GetX() > can_limit);
      const bool y_wrap = (position[i].GetY() > can_limit);
      if (x_wrap) mycanvas.Image(cell, position[i] - offsetX, image_size, image_size);
      if (y_wrap) mycanvas.Image(cell, position[i] - offsetY, image_size, image_size);
      if (x_wrap && y_wrap) mycanvas.Image(cell, position[i] - offsetXY, image_size, image_size);
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
