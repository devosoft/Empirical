#ifndef EMP_POINT_H
#define EMP_POINT_H

#include <cmath>

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  A simple class to track points in 2-Dimensional space.
//

namespace emp {

  class Point {
  private:
    double x;
    double y;

  public:
    Point(const Point & _in) : x(_in.x), y(_in.y) { ; }
    Point(double _x, double _y) : x(_x), y(_y) { ; }
    ~Point() { ; }

    const Point & operator=(const Point & _in) { x = _in.x; y = _in.y; return *this; }

    inline double GetX() const { return x; }
    inline double GetY() const { return y; }

    double Magnitude() const { return sqrt( x*x + y*y ); }

    // Determine a new point, but don't change this one
    Point GetMidpoint(const Point & p2) const { return Point((x+p2.x)/2, (y+p2.y)/2); }
    Point & GetRot90() const { return Point(y, -x); }
    Point & GetRot180() const { return Point(-x, -y); }
    Point & GetRot270() const { return Point(-y, x); }

    Point operator+(const Point & _in) const { return Point(x + _in.x , y + _in.y); }
    Point operator-(const Point & _in) const { return Point(x - _in.x , y - _in.y); }
    Point operator*(double mult) const { return Point(x * mult, y * mult); }
    Point operator/(double div) const { return Point(x / div, y / div); }

    // Modify this point.
    Point & Set(double _x, double _y) { x=_x; y=_y; return *this; }
    Point & Translate(double shift_x, double shift_y) { x += shift_x; y += shift_y; return *this; }
    Point & TranslateX(double shift) { x += shift; return *this; }
    Point & TranslateY(double shift) { y += shift; return *this; }

    Point & operator+=(const Point & _in) { x += _in.x; y += _in.y; return *this; }
    Point & operator-=(const Point & _in) { x -= _in.x; y -= _in.y; return *this; }
    Point & operator*=(double mult) { x *= mult; y *= mult; return *this; }
    Point & operator/=(double mult) { x /= div;  y /= div; return *this; }

    Point & Rot90() { return Set(y, -x); }
    Point & Rot180() { return Set(-x, -y); }
    Point & Rot270() { return Set(-y, x); }

    // Square-roots are slow to calculate; if we're just doing comparisons, square-distance
    // is usualy going to be sufficient.
    double SquareDistance(const Point & _in) const {
      const double x_dist = x - _in.x;
      const double y_dist = y - _in.y;
      return x_dist*x_dist + y_dist*y_dist;
    }

    double Distance(const Point & _in) { return sqrt( SquareDisatance(_in) ); }
  };

};

#endif
