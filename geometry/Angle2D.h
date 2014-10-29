#ifndef EMP_ANGLE_2D_H
#define EMP_ANGLE_2D_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class maintains an angle on a 2D surface.
//
//  The internal representation uses an int to represent angles.
//  First two bytes are number of full circles.
//  Last two bytes represent angle angles.
// 

#include <cmath>

#include "const.h"

namespace emp {

  class Angle {
  private:
    int angle;    // Int representation of an angle

    static const int ANGLE_CAP = 65536;
    static const int UP        = 0;
    static const int RIGHT     = 16384;
    static const int DOWN      = 32768;
    static const int LEFT      = 49152;

  public:
    Angle() : angle(0) { ; }
    Angle(const Angle & in_angle) : angle(in_angle.angle) { ; }
    Angle(double in_radians) : angle(radians * ANGLE_CAP / (2*PI)) { ; }
    Angle(int in_angle, bool okay) : angle(in_angle) { (void) okay; } // directly set internal value

    double AsRadians() const { return ((double) angle) * 2.0 * PI / ((double) ANGLE_CAP); }
    double AsDegrees() const { return ((double) angle) * 360.0 / ((double) ANGLE_CAP); }
    
    Angle & SetRadians(double in_radians) {
      angle = in_radians * ((double) ANGLE_CAP) / (2.0 * PI);
      return *this;
    }
    Angle & SetDegrees(double in_degrees) {
      angle = in_degrees * ((double) ANGLE_CAP) / 360.0;
      return *this;
    }
    Angle & PointUp()    { angle = UP;    return *this; }
    Angle & PointRight() { angle = RIGHT; return *this; }
    Angle & PointDown()  { angle = DOWN;  return *this; }
    Angle & PointLeft()  { angle = LEFT;  return *this; }

    // Chop off full circles
    Angle & Truncate() { angle &= (ANGLE_CAP - 1); return *this; }

    // Count full circles
    int CountFullCircles() { return angle >> 16; }

    // Some basic rotations...
    Angle & RotateRight() { angle += RIGHT; return *this; }
    Angle & RotateLeft()  { angle -= RIGHT; return *this; }
    Angle & RotateUTurn() { angle += DOWN; return *this; }
    Angle & RotateFull(int turns=1)  { angle += turns << 16; return *this; }

    Angle & RotateRadians(double in_radians) {
      angle += in_radians * ((double) ANGLE_CAP) / (2.0 * PI);
      return *this;
    }
    Angle & RotateDegrees(double in_degrees) {
      angle += in_degrees * ((double) ANGLE_CAP) / 360.0;
      return *this;
    }

    Angle & operator=(const Angle & _in) { angle = _in.angle; }
    
    bool operator==(const Angle & _in) const { return angle == _in.angle; }
    bool operator!=(const Angle & _in) const { return angle != _in.angle; }
    bool operator<(const Angle & _in)  const { return angle <  _in.angle; }
    bool operator<=(const Angle & _in) const { return angle <= _in.angle; }
    bool operator>(const Angle & _in)  const { return angle >  _in.angle; }
    bool operator>=(const Angle & _in) const { return angle >= _in.angle; }

    Angle operator+(const Angle & _in) const { return Angle(angle + _in.angle, true); }
    Angle operator-(const Angle & _in) const { return Angle(angle - _in.angle, true); }
    Angle operator*(double _in)          const { return Angle(angle * _in, true); }
    Angle operator*(int _in)             const { return Angle(angle * _in, true); }
    Angle operator/(double _in)          const { return Angle(angle / _in, true); }
    Angle operator/(int _in)             const { return Angle(angle / _in, true); }

    Angle & operator+=(const Angle & _in) { angle += _in.angle; return *this; }
    Angle & operator-=(const Angle & _in) { angle -= _in.angle; return *this; }
    Angle & operator*=(double _in)          { angle *= _in; return *this; }
    Angle & operator*=(int _in)             { angle *= _in; return *this; }
    Angle & operator/=(double _in)          { angle /= _in; return *this; }
    Angle & operator/=(int _in)             { angle /= _in; return *this; }

    double Sin() const { return sin(AsRadians()); } 
    double Cos() const { return cos(AsRadians()); } 
    double Tan() const { return tan(AsRadians()); } 
  }

};


#endif
