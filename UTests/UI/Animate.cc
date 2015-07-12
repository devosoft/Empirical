#include "../../emtools/emfunctions.h"
#include "../../tools/Random.h"
#include "../../UI/Animate.h"
#include "../../UI/canvas_utils.h"
#include "../../UI/UI.h"

namespace UI = emp::UI;

UI::Document doc("emp_base");

int cx = 150;
int cy = 150;
int cr = 50;
int can_size = 300;

void CanvasAnim(double time) {
  (void) time;

  auto & mycanvas = doc.Canvas("can");

  cx+=3;
  if (cx >= can_size + cr) cx -= can_size;

  mycanvas.Clear();
  mycanvas.Circle(cx, cy, cr, "green", "purple");
  if (cx + cr > can_size) mycanvas.Circle(cx-can_size, cy, cr, "green", "purple");
  mycanvas.Refresh();
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

  emp::DelayCall( [anim](){anim->Start();}, 1000 );
}
