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

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  emp::Polygon poly{emp::Point{200,300}};
  emp::Line2D line{{5,5}, {390,390}};
  emp::Circle2D circle{{150, 150}, 50};

  emp::Random random;

  emp::Size2D can_size{800, 400};
  double poly_rot = 0.0;

public:
  MyAnimate() : doc("emp_base") {
    DEBUG_STACK();
    // Draw a simple circle animation on a canvas
    auto test_canvas = doc.AddCanvas(can_size, "can");

    test_canvas.Draw(circle, emp::Palette::BLUE, emp::Palette::MAGENTA);
    targets.push_back(test_canvas);

    // Draw the new polygon.
    poly.AddPoint({60,25}).AddPoint({50,50}).AddPoint({-50,50}).AddPoint({25,40});
    test_canvas.Draw(poly, emp::Palette::GRAY, emp::Palette::BLUE);

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
    DEBUG_STACK();
    auto canvas = doc.Canvas("can");

    // Update the circle position.
    circle += emp::Point{3, 0};
    if (circle.GetX() >= can_size.Width() + circle.GetRadius()) {
      circle.SetX(-50.0);
    }

    // Draw the new circle.
    canvas.Clear();
    canvas.Draw(circle, emp::Palette::BLUE, emp::Palette::MAGENTA);

    // Update the polygon position
    poly_rot += 0.01;
    canvas.SetRotate(poly_rot);
    canvas.Draw(poly, emp::Palette::GRAY, emp::Palette::BLUE);
    canvas.SetRotate(-poly_rot);

    // Update the line.
    canvas.Draw(line);

    // Draw a series of circles along the bottom.
    const double num_colors = 20;
    auto color_map = emp::MakeHSLRange(num_colors, 400.0, 100.0, 100, 100, 20, 100);
    const double radius = can_size.Width() / (2.0 * num_colors) - 1;
    for (size_t i = 0; i < num_colors; i++) {
      double x_pos = (num_colors + (2 * static_cast<double>(i) + 1) * radius);
      emp::Circle circle{{x_pos, 300}, radius};
      canvas.Draw(circle, color_map[i]);
    }

    doc.Text("fps").Redraw();
  }
};

MyAnimate anim;

int main()
{
}

// Local settings for Empecable file checker.
// empecable_words: poly
