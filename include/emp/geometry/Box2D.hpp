/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief A class to manage rectangles in a 2D plane.
 */

#ifndef EMP_GEOMETRY_BOX2D_HPP_INCLUDE
#define EMP_GEOMETRY_BOX2D_HPP_INCLUDE

#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  template <typename TYPE=double> class Box2D {
  private:
    Point2D<TYPE> ul_corner;
    Point2D<TYPE> size;

  public:
    constexpr Box2D(const Point2D<TYPE> & ul_corner, const Point2D<TYPE> & size)
      : ul_corner(ul_corner), size(size) { ; }
    constexpr Box2D(TYPE _x, TYPE _y, TYPE _w, TYPE _h) : ul_corner(_x, _y), size(_w, _h) { ; }
    constexpr Box2D(const Box2D &) = default;

    Box2D & operator=(const Box2D & _in) = default;
    constexpr int operator<=>(const Box2D &) const = default;

    constexpr TYPE GetWidth() const { return size.GetX(); }
    constexpr TYPE GetHeight() const { return size.GetY(); }

    constexpr TYPE GetLeft() const { return ul_corner.GetX(); }
    constexpr TYPE GetRight() const { return ul_corner.GetX() + size.GetX(); }
    constexpr TYPE GetTop() const { return ul_corner.GetY(); }
    constexpr TYPE GetBottom() const { return ul_corner.GetY() + size.GetY(); }

    constexpr TYPE GetArea() const { return GetWidth() * GetHeight(); }
    constexpr TYPE GetPerimeter() const { return 2 * (GetWidth() + GetHeight()); }

    constexpr TYPE SetPosition(const Point2D<TYPE> & in) { ul_corner = in; }
    constexpr TYPE SetSize(const Point2D<TYPE> & in) { size = in; }

    Box2D<TYPE> & Translate(Point2D<TYPE> shift) { ul_corner += shift; return *this; }

    constexpr bool Contains(const Point2D<TYPE> & point) const {
      return point.GetX() > GetLeft() && point.GetX() < GetRight() &&
             point.GetY() > GetTop() && point.GetY() < GetBottom();
    }

    // Is "other" fully contained inside of this box?
    constexpr bool Contains(const Box2D<TYPE> & other) const {
      return other.GetLeft() > GetLeft() && other.GetRight() < GetRight() &&
             other.GetTop() > GetTop() && other.GetBottom() < GetBottom();
    }
    constexpr bool HasOverlap(const Box2D<TYPE> & other) const {
      // Make sure THIS is higher than OTHER.
      if (other.GetTop() < GetTop()) return other.HasOverlap(*this);

      if (other.GetTop() > GetBottom()) return false;  // Other box starts BELOW this box.
      if (other.GetLeft() > GetRight()) return false;  // Other box is to the RIGHT of this one.
      if (other.GetRight() < GetLeft()) return false;  // Other box is to the LEFT of this one.
      return true;
    }
  };

  using Box = Box2D<double>;
}

#endif // #ifndef EMP_GEOMETRY_BOX2D_HPP_INCLUDE
