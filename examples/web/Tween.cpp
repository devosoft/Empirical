/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file Tween.cpp
 */

#include "emp/web/Tween.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;
UI::Document doc("emp_base");
UI::Tween tween(7);

double myvar = 20.0;

double cx = 150;
double cy = 150;
double cr = 50;
double can_size = 400;

void SetVar(double v) { myvar = v; }

void TweenAnim() {
  auto mycanvas = doc.Canvas("can");

  mycanvas.Clear();
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "green", "purple");
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
  auto mycanvas = doc.AddCanvas((size_t) can_size, (size_t) can_size, "can");
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  tween.AddPath(cx, cr, can_size-cr);
  tween.AddPath(cy, cr, can_size-cr);
  tween.AddUpdate(TweenAnim);
}
