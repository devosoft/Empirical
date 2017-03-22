//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "tools/Random.h"
#include "web/Animate.h"
#include "web/canvas_utils.h"
#include "web/emfunctions.h"
#include "web/web.h"

namespace UI = emp::web;

class MyAnimate : public UI::Animate {
private:
  UI::Document doc;
  emp::Random random;

  size_t can_w = 400;
  size_t can_h = 400;

public:
  MyAnimate() : doc("emp_base") {
    // Draw a simple circle animation on a canvas
    auto mycanvas = doc.AddCanvas(can_w, can_h, "can");
    targets.push_back(mycanvas);

    doc << UI::Text("fps") << "FPS = " << UI::Live( [this](){return 1000.0 / GetStepTime();} ) ;
  }

  void DoFrame() {
    auto canvas = doc.Canvas("can");
    canvas.Clear("black");   // <-- or should planets only be drawn once?    

    doc.Text("fps").Redraw();
  }
};

MyAnimate anim;

int main()
{
}
