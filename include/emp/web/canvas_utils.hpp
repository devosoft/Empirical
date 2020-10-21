/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  canvas_utils.hpp
 *  @brief Various versions of the Draw() function to draw images onto a canvas.
 *
 *  Each version of Draw() takes a canvas widget and some form of data to be drawn on the widget,
 *  such as a circle, a bit matrix, or a geometric surface.
 */

#ifndef EMP_WEB_CANVAS_UTILS_H
#define EMP_WEB_CANVAS_UTILS_H

#include "Canvas.hpp"

#include "../base/vector.hpp"
#include "../Evolve/StateGrid.hpp"
#include "../geometry/Circle2D.hpp"
#include "../geometry/Surface2D.hpp"
#include "../geometry/Surface.hpp"
#include "../bits/BitMatrix.hpp"
#include "color_map.hpp"

namespace emp {
namespace web {

  /// Draw a Circle onto the canvas.
  void Draw(Canvas canvas, const emp::Circle & circle,
            const std::string & fill="",
            const std::string & line="")
  {
    canvas.Clear();
    canvas.Draw(circle, fill, line);
  }


  /// Draw a BitMatrix onto a canvas using black and white squares (can specify cell width and height)
  template <size_t COLS, size_t ROWS>
  void Draw(Canvas canvas, const BitMatrix<COLS,ROWS> & matrix, double w, double h)
  {
    canvas.Clear();

    double cell_w = w / (double) COLS;
    double cell_h = h / (double) ROWS;

    for (size_t x = 0; x < COLS; x++) {
      for (size_t y = 0; y < ROWS; y++) {
        if (matrix.Get(x,y)) {
          canvas.Rect({x*cell_w, y*cell_h}, cell_w, cell_h, "black");
        }
      }
    }
  }


  /// Draw a Surface, specifying the full colormap to be used.  The surface has a range of circle
  /// bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  template <typename... BODY_TYPES>
  void Draw(Canvas canvas,
            const Surface<BODY_TYPES...> & surface,
            const emp::vector<std::string> & color_map)
  {
    canvas.Clear();

    const double w = surface.GetWidth();
    const double h = surface.GetHeight();

    // Setup a black background for the surface
    canvas.Rect({0,0}, w, h, "black");

    // Draw the circles.
    const auto & body_set = surface.GetBodySet();
    // RawImage image("images/cell.png");
    for (auto & body : body_set) {
      canvas.Circle(body.center, body.radius, color_map[body.color], "white");
      // canvas.Draw(body->GetPerimeter(), "", color_map[body->GetColorID()]);
      // emp::Circle per = body->GetPerimeter();
      // canvas.Image(image, (size_t) per.GetCenterX(), (size_t) per.GetCenterY(), per.GetRadius()*2.0, per.GetRadius()*2.0);
    }
  }

  /// Draw a Surface, just specifying the number of colors (and using a generated hue map).
  /// The surface has a range of circle bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  /// @param num_colors The number of distinct colors to use in visualization.
  template <typename... BODY_TYPES>
  void Draw(Canvas canvas, const Surface<BODY_TYPES...> & surface, size_t num_colors)
  {
    Draw(canvas, surface, GetHueMap(num_colors));
  }

  // @CAO: THIS IS THE OLD Surface2D to be DEPRICATED!!
  //
  /// Draw a Surface2D, specifying the full colormap to be used.  The surface has a range of circle
  /// bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  template <typename BODY_TYPE>
  void Draw(Canvas canvas,
            const Surface2D<BODY_TYPE> & surface,
            const emp::vector<std::string> & color_map)
  {
    canvas.Clear();

    const double w = surface.GetWidth();
    const double h = surface.GetHeight();

    // Setup a black background for the surface
    canvas.Rect({0,0}, w, h, "black");

    // Draw the circles.
    const auto & body_set = surface.GetConstBodySet();
    // RawImage image("images/cell.png");
    for (auto body : body_set) {
      canvas.Draw(body->GetPerimeter(), color_map[body->GetColorID()], "white");
      canvas.Draw(body->GetPerimeter(), "", color_map[body->GetColorID()]);
      // emp::Circle per = body->GetPerimeter();
      // canvas.Image(image, (size_t) per.GetCenterX(), (size_t) per.GetCenterY(), per.GetRadius()*2.0, per.GetRadius()*2.0);
    }
  }

  /// Draw a Surface2D, just specifying the number of colors (and using a generated hue map).
  /// The surface has a range of circle bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  /// @param num_colors The number of distinct colors to use in visualization.
  template <typename BODY_TYPE>
  void Draw(Canvas canvas, const Surface2D<BODY_TYPE> & surface, size_t num_colors)
  {
    Draw(canvas, surface, GetHueMap(num_colors));
  }


  /// Draw a grid onto a canvas.
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  /// @param cell_width How many pixels wide is each cell to draw?
  /// @param cell_height How many pixels tall is each cell to draw?
  /// @param offset_x How far should we shift the grid relative to the left side of the canvas?
  /// @param offset_y How far should we shift the grid relative to the top of the canvas?
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color,
            double cell_width, double cell_height,
            double offset_x, double offset_y)
  {
    canvas.Clear();

    // Setup a black background for the grid.
    canvas.Rect({0,0}, canvas.GetWidth(), canvas.GetHeight(), "black");

    // Fill out the grid!
    const size_t grid_rows = grid.size();
    const size_t grid_cols = grid[0].size();
    for (size_t row = 0; row < grid_rows; row++) {
      const double cur_y = offset_y + row*cell_height;
      for (size_t col = 0; col < grid_cols; col++) {
        const double cur_x = offset_x + col*cell_width;
        const std::string & cur_color = color_map[grid[row][col]];
        canvas.Rect({cur_x,cur_y}, cell_width, cell_height, cur_color, line_color);
      }
    }
  }

  /// Draw a grid onto a canvas, but without offsets provided -- the grid is centered.
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  /// @param cell_width How many pixels wide is each cell to draw?
  /// @param cell_height How many pixels tall is each cell to draw?
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color,
            double cell_w, double cell_h)
  {
    const double canvas_w = canvas.GetWidth();
    const double canvas_h = canvas.GetHeight();
    const double grid_w = cell_w * grid[0].size();
    const double grid_h = cell_h * grid.size();

    // Center the grid on the canvas if there's extra room.
    const double offset_x = (canvas_w <= grid_w) ? 0 : (canvas_w - grid_w) / 2;
    const double offset_y = (canvas_h <= grid_h) ? 0 : (canvas_h - grid_h) / 2;

    // Call Draw with all of the extra details.
    Draw(canvas, grid, color_map, line_color, cell_w, cell_h, offset_x, offset_y);
  }

  /// Draw a grid onto a canvas, but without cell size provided -- maximize to fill the canvas!
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  void Draw(Canvas canvas,
            const emp::vector<emp::vector<size_t>> & grid,
            const emp::vector<std::string> & color_map,
            std::string line_color="black")
  {
    // Determine the cell width & height
    const double cell_w = canvas.GetWidth() / grid[0].size();
    const double cell_h = canvas.GetHeight() / grid.size();

    Draw(canvas, grid, color_map, line_color, cell_w, cell_h);
  }

  /// Draw a vector onto a canvas as a grid.
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs
  /// @param grid_cols Number of columns in the grid
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid
  /// @param cell_width How many pixels wide is each cell to draw?
  /// @param cell_height How many pixels tall is each cell to draw?
  /// @param offset_x How far should we shift the grid relative to the left side of the canvas?
  /// @param offset_y How far should we shift the grid relative to the top of the canvas?
  void Draw(Canvas canvas,
            const emp::vector<size_t> & grid,
            size_t grid_cols,
            const emp::vector<std::string> & color_map,
            std::string line_color,
            double cell_width, double cell_height,
            double offset_x, double offset_y)
  {
    canvas.Clear();

    // Setup a black background for the grid.
    canvas.Rect({0,0}, canvas.GetWidth(), canvas.GetHeight(), "black");

    // Fill out the grid!
    const size_t grid_rows = grid.size() / grid_cols;
    size_t id = 0;
    for (size_t row = 0; row < grid_rows; row++) {
      const double cur_y = offset_y + row*cell_height;
      for (size_t col = 0; col < grid_cols; col++) {
        const double cur_x = offset_x + col*cell_width;
        const std::string & cur_color = color_map[grid[id++]];
        canvas.Rect({cur_x,cur_y}, cell_width, cell_height, cur_color, line_color);
      }
    }
  }

  /// Draw a state grid onto a canvas.
  /// @param canvas The Canvas to draw on.
  /// @param state_grid A StateGrid object.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  void Draw(Canvas canvas,
            const StateGrid & state_grid,
            const emp::vector<std::string> & color_map,
            std::string line_color="black")
  {
    // Determine the canvas info.
    const double canvas_w = canvas.GetWidth();
    const double canvas_h = canvas.GetHeight();

    // Determine the cell width & height.
    const double cell_w = canvas_w / state_grid.GetWidth();
    const double cell_h = canvas_h / state_grid.GetHeight();

    // Determine the realized grid width and height on the canvas.
    const double grid_w = cell_w * state_grid.GetWidth();
    const double grid_h = cell_h * state_grid.GetHeight();

    // Center the grid on the canvas if there's extra room.
    const double offset_x = (canvas_w <= grid_w) ? 0 : (canvas_w - grid_w) / 2;
    const double offset_y = (canvas_h <= grid_h) ? 0 : (canvas_h - grid_h) / 2;

    canvas.Clear();

    // Setup a black background for the grid.
    canvas.Rect({0,0}, canvas.GetWidth(), canvas.GetHeight(), line_color);

    // Fill out the grid!
    size_t id = 0;
    for (size_t row = 0; row < state_grid.GetHeight(); row++) {
      const double cur_y = offset_y + row*cell_h;
      for (size_t col = 0; col < state_grid.GetWidth(); col++) {
        const double cur_x = offset_x + col*cell_w;
        const int state = state_grid.GetStates()[id++];
        if (state < 0) continue; // leave negative-number squares blank...
        const std::string & cur_color = color_map[(size_t) state];
        canvas.Rect({cur_x,cur_y}, cell_w, cell_h, cur_color, line_color);
      }
    }
  }

  /// Draw a grid as the background of a canvas.
  /// Since this is a BG, clear the canvas first.
  /// @param canvas The Canvas to draw on.
  /// @param rows Number of rows to draw in the grid.
  /// @param cols Number of columns to draw in the grid.
  /// @param bg_color The background color for the grid.
  /// @param line_color The color of the liens on the grid.
  void DrawGridBG(Canvas canvas, size_t rows, size_t cols,
                  const std::string & bg_color, const std::string & line_color) {
    canvas.Clear(bg_color);

    const double canvas_x = (double) canvas.GetWidth();
    const double canvas_y = (double) canvas.GetHeight();
    const double cell_width = canvas_x / cols;
    const double cell_height = canvas_y / rows;

    for (size_t i = 0; i <= cols; i++) {
      double x = cell_width * i;
      canvas.Line( {x,0}, {x,canvas_y}, line_color);
    }
    for (size_t i = 0; i <= rows; i++) {
      double y = cell_height * i;
      canvas.Line( {0,y}, {canvas_x,y}, line_color);
    }
  }

  template <typename CONTAINER_T, typename POINT_FUN_T, typename COLOR_FUN_T>
  void DrawPoints(Canvas canvas, CONTAINER_T && container, double radius,
                  POINT_FUN_T && point_fun, COLOR_FUN_T && color_fun,
                  const std::string & line_color="black")
  {
    // Draw all of the organisms
    for (auto obj : container) {
      const auto pos = point_fun(obj);
      const auto color = color_fun(obj);
      canvas.Circle(pos, radius, color, line_color);
    }
  }


}
}

#endif
