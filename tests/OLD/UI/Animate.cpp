/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Animate.cpp
 */

#include "../../tools/Random.h"
#include "../../UI/Animate.h"
#include "../../UI/canvas_utils.h"
#include "../../UI/UI.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int cx = 150;
int cy = 150;
int cr = 50;
int can_size = 400;

void CanvasAnim(double time) {
  auto & mycanvas = doc.Canvas("can");

  std::cerr << time << std::endl;

  cx+=3;
  if (cx >= can_size + cr) cx -= can_size;

  mycanvas.Clear();
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "green", "purple");
  mycanvas.Refresh();

  doc.Text("fps").Update();
};

int main()
{
  UI::Initialize();

  // How big should each canvas be?
  const int w = can_size;
  const int h = can_size;

  emp::Random random;

  // Draw a simple circle animation on a canvas
  auto & mycanvas = doc.AddCanvas(w, h, "can");
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  doc.Update();


  UI::Animate * anim = new UI::Animate(CanvasAnim, mycanvas);
  (void) anim;

  doc << "<br>";
  // doc.AddButton([anim](){anim->Start();}, "Start");
  // doc.AddButton([anim](){anim->Stop();}, "Pause");
  doc.AddButton([anim](){
      anim->ToggleActive();
      auto & but = doc.Button("toggle");
      if (anim->GetActive()) but.Label("Pause");
      else but.Label("Start");
      but.Update();
    }, "Start", "toggle");

  doc << UI::Text("fps") << "FPS = " << UI::Live( [anim](){return anim->GetStepTime();} ) ;

}
