/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/canvas_utils.hpp
 * @brief Various versions of the Draw() function to draw images onto a canvas.
 *
 * Each version of Draw() takes a canvas widget and some form of data to be drawn on the widget,
 * such as a circle, a bit matrix, or a geometric surface.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_CANVAS_UTILS_HPP_GUARD
#define INCLUDE_EMP_WEB_CANVAS_UTILS_HPP_GUARD

#include <stddef.h>

#include "../base/vector.hpp"
#include "../bits/BitMatrix.hpp"
#include "../Evolve/StateGrid.hpp"
#include "../geometry/Circle2D.hpp"
#include "../geometry/Surface.hpp"

#include "Canvas.hpp"
#include "Color.hpp"
#include "color_map.hpp"

namespace emp::web {

  /// Draw a BitMatrix onto a canvas using black and white squares (can specify cell width and height)
  /// Draw a Circle onto the canvas.
  /// @param canvas The Canvas to draw on.
  /// @param matrix The matrix to draw
  /// @param w The width of the matrix (number of columns)
  /// @param h The height of the matrix (number of rows)
  template <size_t COLS, size_t ROWS>
  void Draw(Canvas canvas, const BitMatrix<COLS, ROWS> & matrix, Size2D size) {
    canvas.Clear();

    Size2D cell_size = size / GridSize{ROWS, COLS};

    for (size_t x = 0; x < COLS; x++) {
      for (size_t y = 0; y < ROWS; y++) {
        if (matrix.Get(x, y)) {
          Box2D box{cell_size.Scale(x, y), cell_size};
          canvas.Draw(box, Palette::BLACK);
        }
      }
    }
  }

  /// Draw a Surface, specifying the full colormap to be used.  The surface has a range of circle
  /// bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  template <typename BODY_T>
  void Draw(Canvas canvas,
            const Surface<BODY_T> & surface) {
    canvas.Clear();

    // Setup a black background for the surface
    Box2D box{{0.0, 0.0}, surface.GetSize()};
    canvas.Draw(box, Palette::BLACK);

    // Draw the circles.
    // RawImage image("images/cell.png");
    for (auto & body : surface.GetBodySet()) {
      if (body.IsActive()) {
        canvas.Draw(body.GetPerimeter(), emp::Color{}, body.GetColor());
      }
      // canvas.Image(image, (size_t) per.GetCenterX(), (size_t) per.GetCenterY(), per.GetRadius()*2.0, per.GetRadius()*2.0);
    }
  }

  /// Draw a Surface, just specifying the number of colors (and using a generated hue map).
  /// The surface has a range of circle bodies, each with a color id.
  /// @param canvas The Canvas to draw on.
  /// @param surface A surface containing a set of shapes to draw.
  /// @param num_colors The number of distinct colors to use in visualization.
  template <typename BODY_T>
  void Draw(Canvas canvas, const Surface<BODY_T> & surface, size_t num_colors) {
    Draw(canvas, surface, MakeHueRange(num_colors));
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
            const emp::vector < emp::vector < size_t >> &grid,
            const emp::vector<Color> & color_map,
            const Color & line_color,
            const Size2D cell_size,
            const Point2D offset) {
    canvas.Clear(Palette::BLACK);

    // Fill out the grid!
    const size_t grid_rows = grid.size();
    const size_t grid_cols = grid[0].size();
    Point2D cur_pos = offset;
    for (size_t row = 0; row < grid_rows; row++) {
      for (size_t col = 0; col < grid_cols; col++) {
        const Color & cur_color = color_map[grid[row][col]];
        canvas.Draw(Box2D{cur_pos, cell_size}, cur_color, line_color);
        cur_pos += Point{cell_size.Width(), 0.0};
      }
      cur_pos.SetX(offset.X());
      cur_pos += Point{0.0, cell_size.Height()};
    }
  }

  /// Draw a grid onto a canvas, but without offsets provided -- the grid is centered.
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  /// @param cell_w How many pixels wide is each cell to draw?
  /// @param cell_h How many pixels tall is each cell to draw?
  void Draw(Canvas canvas,
            const emp::vector < emp::vector < size_t >> & grid,
            const emp::vector<Color> & color_map,
            const emp::Color & line_color,
            Size2D cell_size) {
    const Size2D canvas_size = canvas.GetSize();
    const Size2D grid_size = cell_size.Scale(grid[0].size(), grid.size());

    // Center the grid on the canvas if there's extra room.
    const Point2D offset = ((canvas_size - grid_size) / 2.0).BoundPositive();

    // Call Draw with all of the extra details.
    Draw(canvas, grid, color_map, line_color, cell_size, offset);
  }

  /// Draw a grid onto a canvas, but without cell size provided -- maximize to fill the canvas!
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  void Draw(Canvas canvas,
            const emp::vector < emp::vector < size_t >> & grid,
            const emp::vector<Color> & color_map,
            Color line_color = Palette::BLACK) {
    const Size2D cell_size = canvas.GetSize() / GridSize{grid};

    Draw(canvas, grid, color_map, line_color, cell_size);
  }

  /// Draw a vector onto a canvas as a grid.
  /// @param canvas The Canvas to draw on.
  /// @param grid A vector of vectors of color IDs
  /// @param grid_cols Number of columns in the grid
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid
  /// @param cell_size How big is each cell?
  /// @param offset How far should we shift the grid on the canvas?
  void Draw(Canvas canvas,
            const emp::vector<size_t> & grid,
            size_t grid_cols,
            const emp::vector<Color> & color_map,
            Color line_color,
            Size2D cell_size,
            Point2D offset) {
    canvas.Clear(Palette::BLACK);

    // Fill out the grid!
    const size_t grid_rows = grid.size() / grid_cols;
    size_t id = 0;
    Point2D cell_pos = offset;
    for (size_t row = 0; row < grid_rows; row++) {
      for (size_t col = 0; col < grid_cols; col++) {
        const Color & cur_color = color_map[grid[id++]];
        canvas.Draw(Box2D{cell_pos, cell_size}, cur_color, line_color);
        cell_pos += Point{cell_size.Width(), 0.0};
      }
      cell_pos.Set(offset.X(), cell_pos.Y()+cell_size.Height());
    }
  }

  /// Draw a state grid onto a canvas.
  /// @param canvas The Canvas to draw on.
  /// @param state_grid A StateGrid object.
  /// @param color_map Mapping of values to the colors with which they should be associated.
  /// @param line_color The background line color for the grid.
  void Draw(Canvas canvas,
            const StateGrid & state_grid,
            const emp::vector<Color> & color_map,
            Color line_color = Palette::BLACK) {
    const Size2D canvas_size = canvas.GetSize();
    const Size2D cell_size = canvas_size.ToCellSize(state_grid.NumRows(), state_grid.NumCols());

    canvas.Clear(line_color);

    // Fill out the grid!
    size_t id = 0;
    Point2D cell_pos;
    for (size_t row = 0; row < state_grid.NumRows(); row++) {
      for (size_t col = 0; col < state_grid.NumCols(); col++) {
        const int state = state_grid.GetStates()[id++];
        if (state < 0) continue;  // leave negative squares blank...
        canvas.Draw(Box2D{cell_pos, cell_size}, color_map[state], line_color);
        cell_pos += Point{cell_size.Width(), 0.0};
      }
      cell_pos.Set(0.0, cell_pos.Y() + cell_size.Height());
    }
  }

  /// Draw a grid as the background of a canvas.
  /// Since this is a BG, clear the canvas first.
  /// @param canvas The Canvas to draw on.
  /// @param rows Number of rows to draw in the grid.
  /// @param cols Number of columns to draw in the grid.
  /// @param bg_color The background color for the grid.
  /// @param line_color The color of the liens on the grid.
  void DrawGridBG(Canvas canvas,
                  GridSize grid_size,
                  const Color & bg_color,
                  const Color & line_color) {
    canvas.Clear(bg_color);

    const Size2D canvas_size = canvas.GetSize();
    const Size2D cell_size = canvas_size / grid_size;

    for (size_t i = 0; i <= grid_size.NumCols(); i++) {
      double x = cell_size.Width() * i;
      canvas.Draw(Point{x, 0}, Point{x, canvas_size.Height()}, line_color);
    }
    for (size_t i = 0; i <= grid_size.NumRows(); i++) {
      double y = cell_size.Height() * i;
      canvas.Draw(Point{0, y}, Point{canvas_size.Width(), y}, line_color);
    }
  }

  template <typename CONTAINER_T, typename POINT_FUN_T, typename COLOR_FUN_T>
  void DrawPoints(Canvas canvas,
                  CONTAINER_T && container,
                  const double radius,
                  POINT_FUN_T && point_fun,
                  COLOR_FUN_T && color_fun,
                  const Color & line_color = Palette::BLACK) {
    // Draw all of the organisms
    for (auto obj : container) {
      const auto pos   = point_fun(obj);
      const auto color = color_fun(obj);
      canvas.Draw(Circle{pos, radius}, color, line_color);
    }
  }

}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_CANVAS_UTILS_HPP_GUARD
