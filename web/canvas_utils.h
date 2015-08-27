#ifndef EMP_WEB_CANVAS_UTILS_H
#define EMP_WEB_CANVAS_UTILS_H

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
#include "../tools/vector.h"

namespace emp {
namespace web {

  // Draw a Circle!
  void Draw(Canvas canvas, const emp::Circle<> & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    canvas.Clear();
    canvas.Circle(circle, fill, line);
  }


  // Draw a BitMatrix!
  template <int COLS, int ROWS>
  void Draw(Canvas canvas, const BitMatrix<COLS,ROWS> & matrix, double w, double h)
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

  
  // Draw a Surface2D, specifying the full colormap to be used.
  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE=double>
  void Draw(Canvas canvas,
            const Surface2D<BODY_TYPE,BODY_INFO,BASE_TYPE> & surface,
            const emp::vector<std::string> & color_map)
  {
    canvas.Clear();

    const double w = surface.GetWidth();
    const double h = surface.GetHeight();

    // Setup a black background for the surface
    canvas.Rect(0, 0, w, h, "black");

    // Draw the circles.
    const auto & body_set = surface.GetConstBodySet();
    for (auto * body : body_set) {
      //canvas.Circle(body->GetPerimeter(), color_map[body->GetColorID()], "white");
      canvas.Circle(body->GetPerimeter(), "", color_map[body->GetColorID()]);
      //canvas.Circle(body->GetPerimeter(), "", "white");
    }
  }

  // Draw a Surface2D, just specifying the number of colors.
  template <typename BODY_TYPE, typename BODY_INFO, typename BASE_TYPE=double>
  void Draw(Canvas canvas,
            const Surface2D<BODY_TYPE,BODY_INFO,BASE_TYPE> & surface,
            int num_colors)
  {
    Draw(canvas, surface, GetHueMap(num_colors));
  }


  // Draw a grid.
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<int>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color="black",
            bool square_cells=true,
            int cell_width=-1,
            int cell_height=-1,
            bool auto_offsets=true,
            int offset_x=0,
            int offset_y=0) {
    canvas.Clear();
    const int canvas_width = canvas.GetWidth();
    const int canvas_height = canvas.GetHeight();
    const int grid_rows = (int) grid.size();
    const int grid_cols = (int) grid[0].size();

    // Determine the cell width & height
    if (cell_width < 0) cell_width = canvas_width / grid_cols;
    if (cell_height < 0) cell_height = canvas_height / grid_rows;

    if (square_cells && cell_width != cell_height) {
      cell_width = std::min(cell_width, cell_height);
      cell_height = cell_width;
    }


    // Determine the offsets to center the grid.
    if (auto_offsets) {
      offset_x = (canvas_width - grid_cols * cell_width) / 2;
      offset_y = (canvas_height - grid_rows * cell_height) / 2;
    }

    // Setup a black background for the grid.
    canvas.Rect(0, 0, canvas_width, canvas_height, "black");

    // Fill out the grid!
    for (int row = 0; row < grid_rows; row++) {
      const int cur_y = offset_y + row*cell_height;
      for (int col = 0; col < grid_cols; col++) {
        const int cur_x = offset_x + col*cell_width;
        const std::string & cur_color = color_map[grid[row][col]];
        canvas.Rect(cur_x, cur_y, cell_width, cell_height, cur_color, line_color);
      }
    }

  }

}
}

#endif
