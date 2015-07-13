#ifndef EMP_UI_CANVAS_UTILS_H
#define EMP_UI_CANVAS_UTILS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Method for converting objects to visualizations in canvas.
//
//  Current list of values that can be drawn with Draw():
//  * BitMatrix
//  * Circle
//  * Surface2D
//

#include "Canvas.h"

#include "../emtools/color_map.h"
#include "../geometry/Circle2D.h"
#include "../geometry/Surface2D.h"
#include "../tools/BitMatrix.h"

namespace emp {
namespace UI {

  // Draw a Circle!
  void Draw(Canvas & canvas, const emp::Circle<> & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    canvas.Clear();

    canvas.Circle(circle, fill, line);
  }


  // Draw a BitMatrix!
  template <int COLS, int ROWS>
  void Draw(Canvas & canvas, const BitMatrix<COLS,ROWS> & matrix, double w, double h)
  {
    canvas.Clear();

    double cell_w = w / (double) COLS;
    double cell_h = h / (double) ROWS;

    for (int x = 0; x < COLS; x++) {
      for (int y = 0; y < ROWS; y++) {
        if (matrix.Get(x,y)) {
          canvas.Rect(x*cell_w, y*cell_h, cell_w, cell_h, "black");
        }
      }
    }
  }

  
  // Draw a Surface2D!
  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE=double>
  void Draw(Canvas & canvas,
            const Surface2D<BODY_TYPE,BODY_INFO,BASE_TYPE> & surface,
            const std::vector<std::string> & color_map)
  {
    canvas.Clear();

    const double w = surface.GetWidth();
    const double h = surface.GetHeight();

    // Setup a black background for the surface.
    canvas.Rect(0, 0, w, h, "black");

    // Draw the circles.
    const auto & body_set = surface.GetConstBodySet();
    for (auto * body : body_set) {
      canvas.Circle(body->GetPerimeter(), color_map[body->GetColorID()], "white");
    }
  }

  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE=double>
  void Draw(Canvas & canvas,
            const Surface2D<BODY_TYPE,BODY_INFO,BASE_TYPE> & surface,
            int num_colors)
  {
    Draw(canvas, surface, GetHueMap(num_colors));
  }

  
}
}

#endif
