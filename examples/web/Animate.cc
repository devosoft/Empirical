// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include "../../emtools/emfunctions.h"
#include "../../tools/Random.h"
#include "../../web/Animate.h"
#include "../../web/canvas_utils.h"
#include "../../web/web.h"

namespace web = emp::web;

web::Document doc("emp_base");

int cx = 150;
int cy = 150;
int cr = 50;
int can_size = 400;

void CanvasAnim(double time) {
  auto mycanvas = doc.Canvas("can");

  std::cerr << time << std::endl;

  cx+=3;
  if (cx >= can_size + cr) cx -= can_size;

  mycanvas.Clear();
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "green", "purple");

  doc.Text("fps").Redraw();
};

int main()
{
  // How big should each canvas be?
  const int w = can_size;
  const int h = can_size;

  emp::Random random;

  // Draw a simple circle animation on a canvas
  auto mycanvas = doc.AddCanvas(w, h, "can");
  mycanvas.Circle(cx, cy, cr, "green", "purple");

  web::Animate * anim = new web::Animate(CanvasAnim, mycanvas);
  (void) anim;

  doc << "<br>";
  doc.AddButton([anim](){
      anim->ToggleActive();
      auto but = doc.Button("toggle");
      if (anim->GetActive()) but.Label("Pause");
      else but.Label("Start");
    }, "Start", "toggle");

  doc << web::Text("fps") << "FPS = " << web::Live( [anim](){return anim->GetStepTime();} ) ;

}
