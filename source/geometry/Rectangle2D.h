//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A class to manage circles in a 2D plane.

// TODO(discuss): @amlalejini: Had to nuke constexpr to make inheritance from Shape2D work.
//    -- But, it *is* nice to have Circle2D class that can be constructed and operated on at compile time.
//    -- What do?

#ifndef EMP_RECT_2D_H
#define EMP_RECT_2D_H

#include "Shape2D.h"
#include "Point2D.h"

#include "base/assert.h"
template <typename T>
class Circle2D;

template <typename T>
class Line2D;

namespace emp {
  // @ELD: TODO: Pretty sure a lot of this breaks if TYPE is int
  template <typename TYPE=double> class Rect2D : public Shape2D {
  private:
    Point2D<TYPE> center;
    Point2D<TYPE> ul; // upper left
    Point2D<TYPE> lr; // lower right
    TYPE radius; //radius of bounding circle
    TYPE height;
    TYPE width;

  public:
    Rect2D(const Point2D<TYPE> & _ul, const Point2D<TYPE> & _lr) :
                                      ul(_ul), lr(_lr) {
        center = ul.GetMidpoint(lr);
        radius = center.Distance(ul);
        width = ul.XDistance(lr);
        height = ul.YDistance(lr);
    }
    Rect2D(const Rect2D<TYPE> & _rect) : center(_rect.center), ul(_rect.ul),
                                         lr(_rect.lr),
                                         radius(_rect.radius),
                                         height(_rect.height),
                                         width(_rect.width){;}
    // Rectangle2D(TYPE _x, TYPE _y, TYPE _r=0) : center(_x,_y), radius(_r) { ; }
    // Rectangle2D(TYPE _r=0) : center(0.0, 0.0), radius(_r) { ; }

    const Point2D<TYPE> & GetCenter() const override { return center; }
    TYPE GetCenterX() const override { return center.GetX(); }
    TYPE GetCenterY() const override { return center.GetY(); }
    TYPE GetRadius() const override { return radius; }
    TYPE GetSquareRadius() const override {  return radius * radius; }
    TYPE GetULX() const {return ul.GetX();}
    TYPE GetULY() const {return ul.GetY();}
    TYPE GetLRX() const {return lr.GetX();}
    TYPE GetLRY() const {return lr.GetY();}
    TYPE GetWidth() const {return width;}
    TYPE GetHeight() const {return height;}

    Line2D<TYPE> GetTop() const {return Line2D<TYPE>(ul, Point(lr.GetX(), ul.GetY()));}
    Line2D<TYPE> GetBottom() const {return Line2D<TYPE>(lr, Point(ul.GetX(), lr.GetY()));}
    Line2D<TYPE> GetLeft() const {return Line2D<TYPE>(ul, Point(ul.GetX(), lr.GetY()));}
    Line2D<TYPE> GetRight() const {return Line2D<TYPE>(lr, Point(lr.GetX(), ul.GetY()));}

    Rect2D<TYPE> & SetCenter(const Point2D<TYPE> & c) override {
        center = c;
        ul.SetX(center.GetX()-(width/2));
        ul.SetY(center.GetY()-(height/2));
        lr.SetX(center.GetX()+(width/2));
        lr.SetY(center.GetY()+(height/2));

        return *this;
    }

    Rect2D<TYPE> & SetCenter(TYPE x, TYPE y) {
        center.Set(x,y);
        ul.SetX(x-(width/2));
        ul.SetY(y-(height/2));
        lr.SetX(x+(width/2));
        lr.SetY(y+(height/2));

        return *this;
    }

    Rect2D<TYPE> & SetCenterX(TYPE x) override {
        center.SetX(x);
        ul.SetX(x-(width/2));
        lr.SetX(x+(width/2));

        return *this;
    }
    Rect2D<TYPE> & SetCenterY(TYPE y) override {
        center.SetY(y);
        ul.SetY(y-(height/2));
        lr.SetY(y+(height/2));

        return *this;
    }

    // @ELD: Should we really allow setting the radius of a rectangle?
    Rect2D<TYPE> & SetRadius(TYPE new_radius) override {
        double scale_factor = new_radius/radius;
        radius = new_radius;
        width *= scale_factor;
        height *= scale_factor;
        ul.SetX(center.GetX() - (width/2));
        ul.SetY(center.GetY() + (height/2));
        lr.SetX(center.GetX() + (width/2));
        lr.SetY(center.GetY() - (height/2));

        return *this;
    }

    // Circle2D<TYPE> & Set(const Point2D<TYPE> & c, TYPE r) { center = c; radius = r; return *this; }
    // Circle2D<TYPE> & Set(TYPE x, TYPE y, TYPE r) { center.Set(x,y); radius = r; return *this; }

    Rect2D<TYPE> & Translate(Point2D<TYPE> shift) override {
        center += shift;
        ul += shift;
        lr += shift;
        return *this;
    }

    bool Contains(const Point2D<TYPE> & point) const {
        return point.GetX() > ul.GetX() && point.GetX() < lr.GetX()
                    && point.GetY() < ul.GetY() && point.GetY() > lr.GetY();
    }
    bool Contains(TYPE x, TYPE y) const {
        return x > ul.GetX() && x < lr.GetX()
                    && y < ul.GetY() && y > lr.GetY();
    }
    bool Contains(const Rect2D<TYPE> & other) const {
      return other.ul.GetX() > ul.GetX() && other.ul.GetY() < ul.GetY()
                && other.lr.GetX() < lr.GetX() && other.lr.GetY() > lr.GetY();
    }
    bool Contains(const Circle2D<TYPE> & other) const {
      if (ul.GetX() + width == lr.GetX()) {
          // Rect is not rotated
          return (other.GetCenterX() + other.GetRadius() > ul.GetX() &&
                 other.GetCenterX() - other.GetRadius() < lr.GetX()) &&
                 (other.GetCenterY() - other.GetRadius() < ul.GetY &&
                 other.GetCenterY() + other.GetRadius() > ul.GetY());
      }
      emp_assert(false, "NOT IMPLEMENTED");
      return false;
    }

    bool HasOverlap(const Rect2D<TYPE> & other) const {
      return ul.GetX() < other.lr.GetX() && lr.GetY() > other.ul.GetY()
        && ul.GetY() > other.lr.GetY() && lr.GetY() < other.ul.GetY();
    }
    bool HasOverlap(const Circle2D<TYPE> & other) const {
      if (ul.GetX() + width == lr.GetX()) {
          // Rect is not rotated
          // I'm worried that there's a literal corner case this is missing
          return (other.GetCenterX() + other.GetRadius() > ul.GetX() &&
                 other.GetCenterX() - other.GetRadius() < lr.GetX()) &&
                 (other.GetCenterY() + other.GetRadius() > ul.GetY() &&
                 other.GetCenterY() - other.GetRadius() < lr.GetY());
      }
      emp_assert(false, "NOT IMPLEMENTED");
      return false;
    }

  };

  using Rect = Rect2D<double>;
}

#endif
