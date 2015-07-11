#ifndef EMP_UI_CANVAS_UTILS_H
#define EMP_UI_CANVAS_UTILS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Method for converting objects to visualizations in canvas.
//

#include "Canvas.h"

#include "../geometry/Circle2D.h"
#include "../tools/BitMatrix.h"

namespace emp {
namespace UI {

  void Draw(Canvas & canvas, const Circle<> & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    canvas.Circle(circle, fill, line);
  }

  template <int COLS, int ROWS>
  void Draw(Canvas & canvas, const BitMatrix<COLS,ROWS> & matrix, double w, double h)
  {
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

}
}

#endif
