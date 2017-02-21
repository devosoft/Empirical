//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Specs for the CanvasShape widget and various specific shapes:
//
//    CanvasCircle
//    CanvasRect


#ifndef EMP_WEB_CANVAS_SHAPE_H
#define EMP_WEB_CANVAS_SHAPE_H

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

    void MoveTo(double _x, double _y) { x=_x; y=_y; }
    void SetFillColor(const std::string & color) { fill_color = color; }
    void SetLineColor(const std::string & color) { line_color = color; }
  };

  class CanvasCircle : public CanvasShape {
    double radius;
  public:
    CanvasCircle(double _x, double _y, double _r,
                 const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc), radius(_r) { ; }

    CanvasCircle(emp::Circle circle, const std::string & fc="", const std::string & lc="")
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
    CanvasAction * Clone() const { return new CanvasCircle(*this); }
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
    CanvasAction * Clone() const { return new CanvasRect(*this); }
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
    CanvasAction * Clone() const { return new CanvasClearRect(*this); }
  };

  class CanvasPolygon : public CanvasShape {
  private:
    emp::vector<Point> points;
  public:
    CanvasPolygon(const std::string & fc="", const std::string & lc="")
      : CanvasShape(0, 0, fc, lc) { ; }
    CanvasPolygon(const emp::vector<Point> & p, const std::string & fc="", const std::string & lc="")
      : CanvasShape(0, 0, fc, lc), points(p) { ; }
    CanvasPolygon(double _x, double _y, const std::string & fc="", const std::string & lc="")
      : CanvasShape(_x, _y, fc, lc) { ; }

    CanvasPolygon & AddPoint(double x, double y) { points.emplace_back(x,y); return *this; }
    CanvasPolygon & AddPoint(Point p) { points.emplace_back(p); return *this; }

    void Apply() {
      EM_ASM_ARGS({
        emp_i.ctx.translate($0,$1);
        emp_i.ctx.beginPath();
        emp_i.ctx.moveTo($2, $3);
      }, x, y, points[0].GetX(), points[0].GetY());  // Setup the polygon
      for (size_t i = 1; i < points.size(); i++) {
        EM_ASM_ARGS({
          emp_i.ctx.lineTo($0, $1);
        }, points[i].GetX(), points[i].GetY());  // Draw the lines for the polygon
      }
      EM_ASM_ARGS({
        emp_i.ctx.closePath();
        emp_i.ctx.translate($0,$1);
      }, -x, -y);  // Close the polygon
      if (fill_color.size()) Fill(fill_color);
      if (line_color.size()) Stroke(line_color);
    }
    CanvasAction * Clone() const { return new CanvasPolygon(*this); }
  };

}
}

#endif
