#ifndef EMP_WEB_CANVAS_SHAPE_H
#define EMP_WEB_CANVAS_SHAPE_H

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
namespace web {

  class CanvasShape : public CanvasAction {
  protected:
    double x; double y;
    std::string fill_color;
    std::string line_color;
  public:
    CanvasShape(double _x, double _y, const std::string & fc="", const std::string & lc="")
      : x(_x), y(_y), fill_color(fc), line_color(lc) { ; }
    virtual ~CanvasShape() { ; }
  };

  class CanvasCircle : public CanvasShape {
    double radius;
  public:
    CanvasCircle(double _x, double _y, double _r,
                 const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc), radius(_r) { ; }

    CanvasCircle(emp::Circle<> circle,
                 const std::string & fc="", const std::string & lc="")
      : CanvasShape(circle.GetCenterX(), circle.GetCenterY(), fc, lc)
      , radius(circle.GetRadius()) { ; }

    void Apply() {
      EM_ASM_ARGS({
          emp_i.ctx.beginPath();
          emp_i.ctx.arc($0, $1, $2, 0, Math.PI*2);
        }, x, y, radius);  // Draw the circle
      if (fill_color.size()) Fill(fill_color);
      if (line_color.size()) Stroke(line_color);
    }
    CanvasAction * Clone() { return new CanvasCircle(*this); }
  };

  class CanvasRect : public CanvasShape {
    double w; double h;
  public:
    CanvasRect(double _x, double _y, double _w, double _h,
               const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc), w(_w), h(_h) { ; }

    void Apply() {
      EM_ASM_ARGS({
          emp_i.ctx.beginPath();
          emp_i.ctx.rect($0, $1, $2, $3);
        }, x, y, w, h);  // Draw the rectangle
      if (fill_color.size()) Fill(fill_color);
      if (line_color.size()) Stroke(line_color);
    }
    CanvasAction * Clone() { return new CanvasRect(*this); }
  };

  class CanvasClearRect : public CanvasShape {
    double w; double h;
  public:
    CanvasClearRect(double _x, double _y, double _w, double _h)
      : CanvasShape(_x, _y), w(_w), h(_h) { ; }

    void Apply() {
      EM_ASM_ARGS({
          emp_i.ctx.clearRect($0, $1, $2, $3);
        }, x, y, w, h);  // Draw the rectangle
    }
    CanvasAction * Clone() { return new CanvasClearRect(*this); }
  };

}
}

#endif
