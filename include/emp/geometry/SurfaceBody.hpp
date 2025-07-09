/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Physics2D.hpp
 * @brief Physics2D - handles movement and collisions in a simple 2D world.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_SURFACE_BODY_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_SURFACE_BODY_HPP_GUARD

#include "../math/constants.hpp"

#include "Circle2D.hpp"
#include "Point2D.hpp"
#include "Surface.hpp"

namespace emp {

  class SurfaceBody {
  protected:
    static constexpr size_t NO_ID = MAX_SIZE_T;

    size_t id;         ///< Index in body_set to find body info
    Circle perimeter;  ///< Outer shape for body
    Color color;       ///< What color should this body be?

  public:
    SurfaceBody(size_t id = NO_ID, Circle circle = Circle{1.0}, Color color = Palette::BLACK)
      : id(id), perimeter(circle), color(color) {}

    SurfaceBody(SurfaceBody &&) = default;  // Move constructor.

    ~SurfaceBody() { Deactivate(); }

    SurfaceBody & operator=(SurfaceBody &&) = default;

    [[nodiscard]] size_t GetID() const { return id; }
    [[nodiscard]] bool IsActive() const { return id != NO_ID; }
    void Deactivate() { id = NO_ID; }

    [[nodiscard]] const Circle & GetPerimeter() const { return perimeter; }
    [[nodiscard]] const Point & GetCenter() const { return perimeter.GetCenter(); }
    [[nodiscard]] double GetRadius() const { return perimeter.GetRadius(); }
    [[nodiscard]] const Color & GetColor() const { return color; }

    void MoveTo(const Point & s) { perimeter.SetCenter(s); }
    void MoveBy(const Point & s) { perimeter += s; }
    void SetRadius(double r) { perimeter.SetRadius(r); }
  };
}

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_SURFACE_BODY_HPP_GUARD
