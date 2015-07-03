#ifndef EMP_UI_CANVAS_SHAPE_H
#define EMP_UI_CANVAS_SHAPE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the CanvasShape widget and various specific shapes:
//
//    CanvasCircle
//    CanvasRect
//


#include <string>

#include "CanvasAction.h"

namespace emp {
namespace UI {

  class CanvasShape : public CanvasAction {
  protected:
    int x; int y;
    std::string fill_color;
    std::string line_color;
  public:
    CanvasShape(int _x, int _y, const std::string & fc="", const std::string & lc="")
      : x(_x), y(_y), fill_color(fc), line_color(lc) { ; }
    virtual ~CanvasShape() { ; }
  };

  class CanvasCircle : public CanvasShape {
    int radius;
  public:
    CanvasCircle(int _x, int _y, int _r, const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc), radius(_r) { ; }

    void Apply() {
      EM_ASM_ARGS({
          emp.ctx.beginPath();
          emp.ctx.arc($0, $1, $2, 0, Math.PI*2);
        }, x, y, radius);  // Draw the circle
      Fill(fill_color);
      Stroke(line_color);
    }
    CanvasAction * Clone() { return new CanvasCircle(*this); }
  };

  class CanvasRect : public CanvasShape {
    int w; int h;
  public:
    CanvasRect(int _x, int _y, int _w, int _h, const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc), w(_w), h(_h) { ; }

    void Apply() {
      EM_ASM_ARGS({
          emp.ctx.beginPath();
          emp.ctx.rect($0, $1, $2, $3);
        }, x, y, w, h);  // Draw the rectangle
      Fill(fill_color);
      Stroke(line_color);
    }
    CanvasAction * Clone() { return new CanvasRect(*this); }
  };

}
}

#endif
