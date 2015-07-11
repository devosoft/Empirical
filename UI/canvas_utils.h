#ifndef EMP_UI_CANVAS_UTILS_H
#define EMP_UI_CANVAS_UTILS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Method for converting objects to visualizations in canvas.
//

#include "Canvas.h"
#include "../geometry/Circle2D.h"

namespace emp {
namespace UI {

  template <typename UNIT_TYPE>
  void Draw(Canvas & canvas, const Circle<UNIT_TYPE> & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    const Point<UNIT_TYPE> & center = circle.GetCenter();
    UNIT_TYPE radius = circle.GetRadius();
    canvas.Circle(center.GetX(), center.GetY(), radius, fill, line);
  }

}
}

#endif
