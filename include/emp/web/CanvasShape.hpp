/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/CanvasShape.hpp
 * @brief Setup shapes from emp/geometry/ with colors to draw on a canvas.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_CANVAS_SHAPE_HPP_GUARD
#define INCLUDE_EMP_WEB_CANVAS_SHAPE_HPP_GUARD

#include <stddef.h>
#include <string>

#include "../geometry/Box2D.hpp"
#include "../geometry/Circle2D.hpp"
#include "../geometry/Line2D.hpp"
#include "../geometry/Point2D.hpp"
#include "../geometry/Polygon.hpp"

#include "Color.hpp"

namespace emp::web {

  class LineInfo {
  private:
    double width;
  public:
    constexpr LineInfo(double width=1.0) : width(width) { }
    [[nodiscard]] constexpr double LineWidth() const { return width; }
    [[nodiscard]] constexpr double & LineWidth() { return width; }
  };

  class ColorInfo {
  private:
    Color color;
  public:
    ColorInfo(const Color & color) : color(color) {}
    [[nodiscard]] constexpr Color GetColor() const { return color; }
    [[nodiscard]] constexpr Color & GetColor() { return color; }
  };

  class CanvasBox : public emp::Box2D, public ColorPair, public LineInfo {
  public:
    CanvasBox(Point2D ul_corner, Size2D size,
      ColorPair colors=ColorPair{Palette::BLACK, Palette::NONE},
      double line_width=1.0) : Box2D(ul_corner, size), ColorPair(colors), LineInfo(line_width) {}
  };

  class CanvasCircle : public emp::Circle2D, public ColorPair, public LineInfo {
  public:
    CanvasCircle(Point2D center, double radius,
      ColorPair colors=ColorPair{Palette::BLACK, Palette::NONE},
      double line_width=1.0) : Circle2D(center, radius), ColorPair(colors), LineInfo(line_width) {}
  };

  class CanvasLine : public emp::Line2D, public ColorInfo, public LineInfo {
  public:
    CanvasLine(Point2D start, Point2D end_offset,
      Color color=Palette::BLACK,
      double line_width=1.0) : Line2D(start, end_offset), ColorInfo(color), LineInfo(line_width) {}
  };

  class CanvasPolygon : public emp::Polygon, public ColorPair, public LineInfo {
  public:
    CanvasPolygon(Point2D start_point, const emp::vector<Point2D> & other_pts = emp::vector<Point2D>{},
      ColorPair colors=ColorPair{Palette::BLACK, Palette::NONE},
      double line_width=1.0)
    : Polygon(start_point, other_pts), ColorPair(colors), LineInfo(line_width) {}
  };
}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_CANVAS_SHAPE_HPP_GUARD
