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

#include "../debug/debug.hpp"
#include "../math/constants.hpp"

#include "Circle2D.hpp"
#include "Point2D.hpp"
#include "Surface.hpp"

namespace emp {

  class SurfaceBody {
  private:
    static constexpr size_t NO_ID = MAX_SIZE_T;
    size_t id = NO_ID; ///< Index in body_set to find body info

  protected:
    Circle perimeter;  ///< Outer shape for body
    Color color;       ///< What color should this body be?

  public:
    SurfaceBody(Circle circle = Circle{1.0}, Color color = Palette::BLACK)
      : perimeter(circle), color(color) {}

    // Move constructor.
    SurfaceBody(SurfaceBody && in) : id(in.id), perimeter(in.perimeter), color(in.color) {
      in.id = NO_ID; // Deactivate old version.
    }

    ~SurfaceBody() { Deactivate(); }

    SurfaceBody & operator=(SurfaceBody &&) = default;

    // Track body version with surface version to ensure no mis-matches.
    #ifndef NDEBUG
    mutable int version = -1;
    mutable emp::Ptr<const int> surface_version_ptr = nullptr;
    void SetVersion(const int & in_version) const {
      version = in_version;
      surface_version_ptr = &in_version;
    }
    #endif

    [[nodiscard]] bool IsActive() const {
      emp_assert(surface_version_ptr && version == *surface_version_ptr,
                 version, *surface_version_ptr, DebugStackToString());
      return id != NO_ID;
    }

    [[nodiscard]] size_t GetID() const {
      emp_assert(IsActive());
      return id;
    }
    void Activate(size_t in_id) {
      emp_assert(id == NO_ID);
      emp_assert(in_id != NO_ID);
      emp_assert(surface_version_ptr && version == *surface_version_ptr);
      id = in_id;
    }
    void Deactivate() { id = NO_ID; }

    [[nodiscard]] double X() const {
      emp_assert(IsActive());
      return perimeter.GetX();
    }
    [[nodiscard]] double Y() const {
      emp_assert(IsActive());
      return perimeter.GetY();
    }
    [[nodiscard]] double & X() {
      emp_assert(IsActive());
      return perimeter.GetX();
    }
    [[nodiscard]] double & Y() {
      emp_assert(IsActive());
      return perimeter.GetY();
    }

    [[nodiscard]] const Circle & GetPerimeter() const {
      emp_assert(IsActive());
      return perimeter;
    }
    [[nodiscard]] const Point & GetCenter() const {
      emp_assert(IsActive());
      return perimeter.GetCenter();
    }
    [[nodiscard]] double GetRadius() const {
      emp_assert(IsActive());
      return perimeter.GetRadius();
    }
    [[nodiscard]] const Color & GetColor() const {
      emp_assert(IsActive());
      return color;
    }

    [[nodiscard]] Point & GetCenter() {
      emp_assert(IsActive());
      return perimeter.GetCenter();
    }

    void MoveTo(const Point & s) {
      emp_assert(IsActive());
      perimeter.SetCenter(s);
    }
    void MoveBy(const Point & s) {
      emp_assert(IsActive());
      perimeter += s;
    }
    void SetRadius(double r) {
      emp_assert(IsActive());
      perimeter.SetRadius(r);
    }
    void SetColor(Color in) {
      emp_assert(IsActive());
      color = in;
    }
  };
}

#endif  // #ifndef INCLUDE_EMP_GEOMETRY_SURFACE_BODY_HPP_GUARD
