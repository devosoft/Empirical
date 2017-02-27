//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A collection of Draw() functions that convert objects to visualizations in canvas.
//
//  Current list of values that can be drawn with Draw():
//  * BitMatrix
//  * Circle
//  * Surface2D

#ifndef EMP_WEB_CANVAS_UTILS_H
#define EMP_WEB_CANVAS_UTILS_H

#include "Canvas.h"

#include "../base/vector.h"
#include "../geometry/Circle2D.h"
#include "../geometry/Surface2D.h"
#include "../tools/BitMatrix.h"
#include "color_map.h"

namespace emp {
namespace web {

  // Draw a Circle!
  void Draw(Canvas canvas, const emp::Circle & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    canvas.Clear();
    canvas.Circle(circle, fill, line);
  }


  // Draw a BitMatrix!
  template <size_t COLS, size_t ROWS>
  void Draw(Canvas canvas, const BitMatrix<COLS,ROWS> & matrix, double w, double h)
  {
    canvas.Clear();

    double cell_w = w / (double) COLS;
    double cell_h = h / (double) ROWS;

    for (size_t x = 0; x < COLS; x++) {
      for (size_t y = 0; y < ROWS; y++) {
        if (matrix.Get(x,y)) {
          canvas.Rect(x*cell_w, y*cell_h, cell_w, cell_h, "black");
        }
      }
    }
  }


  // Draw a Surface2D, specifying the full colormap to be used.
  template <typename BODY_TYPE>
  void Draw(Canvas canvas,
            const Surface2D<BODY_TYPE> & surface,
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
  template <typename BODY_TYPE>
  void Draw(Canvas canvas, const Surface2D<BODY_TYPE> & surface, size_t num_colors)
  {
    Draw(canvas, surface, GetHueMap(num_colors));
  }


  // Draw a grid.
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color,
            size_t cell_width, size_t cell_height,
            size_t offset_x, size_t offset_y)
  {
    canvas.Clear();

    // Setup a black background for the grid.
    canvas.Rect(0, 0, canvas.GetWidth(), canvas.GetHeight(), "black");

    // Fill out the grid!
    const size_t grid_rows = grid.size();
    const size_t grid_cols = grid[0].size();
    for (size_t row = 0; row < grid_rows; row++) {
      const size_t cur_y = offset_y + row*cell_height;
      for (size_t col = 0; col < grid_cols; col++) {
        const size_t cur_x = offset_x + col*cell_width;
        const std::string & cur_color = color_map[grid[row][col]];
        canvas.Rect(cur_x, cur_y, cell_width, cell_height, cur_color, line_color);
      }
    }
  }

  // If offset is not provided for Draw, calculate it to CENTER the grid.
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color,
            size_t cell_w, size_t cell_h)
  {
    const size_t canvas_w = canvas.GetWidth();
    const size_t canvas_h = canvas.GetHeight();
    const size_t grid_w = cell_w * grid[0].size();
    const size_t grid_h = cell_h * grid.size();

    // Center the grid on the canvas if there's extra room.
    const size_t offset_x = (canvas_w <= grid_w) ? 0 : (canvas_w - grid_w) / 2;
    const size_t offset_y = (canvas_h <= grid_h) ? 0 : (canvas_h - grid_h) / 2;

    // Call Draw with all of the extra details.
    Draw(canvas, grid, color_map, line_color, cell_w, cell_h, offset_x, offset_y);
  }

  // If cell width and height are not provided for grid Draw, calculate largest possible.
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color="black")
  {
    // Determine the cell width & height
    const size_t cell_w = canvas.GetWidth() / grid[0].size();
    const size_t cell_h = canvas.GetHeight() / grid.size();

    Draw(canvas, grid, color_map, line_color, cell_w, cell_h);
  }

}
}

#endif
