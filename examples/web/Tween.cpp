/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2025
*/
/**
 *  @file
 */

#include "emp/web/Tween.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
UI::Document doc("emp_base");
UI::Tween tween(7);

double myvar = 20.0;

emp::Circle circle{{150, 150}, 50};
emp::Size2D can_size{400, 400};

void SetVar(double v) { myvar = v; }

void TweenAnim() {
  auto mycanvas = doc.Canvas("can");

  mycanvas.Clear();
  mycanvas.Draw(circle, emp::Palette::GREEN, emp::Palette::PURPLE);
  if (!can_size.Contains(circle.RightPoint())) {
    mycanvas.Draw(circle - emp::Point{can_size, 0}, emp::Palette::GREEN, emp::Palette::PURPLE);
  }
};

int main()
{
  UI::Text text("text");
  text.SetCSS("border", "3px solid blue").SetCSS("padding", "3px") << "Testing.  "
                                                                   << UI::Live(myvar);
  doc << text;

  tween.AddPath(SetVar, 0, 1000);
  tween.AddDependant(text);

  doc << UI::Button([](){tween.Start();}, "Start!");

  doc << "<br>";
  auto mycanvas = doc.AddCanvas(can_size, "can");
  mycanvas.Draw(circle, emp::Palette::GREEN, emp::Palette::PURPLE);

  // Set a path for the circle to go from the upper left to the lower right.
  tween.AddPath(circle.GetX(), circle.GetRadius(), can_size.Width() - circle.GetRadius());
  tween.AddPath(circle.GetY(), circle.GetRadius(), can_size.Height() - circle.GetRadius());
  tween.AddUpdate(TweenAnim);
}

// Local settings for Empecable file checker.
// empecable_words: myvar
