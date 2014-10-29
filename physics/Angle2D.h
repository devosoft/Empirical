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

#include <math.h>

#include "const.h"

namespace emp {

  class Angle2D {
  private:
    int angle;    // Int representation of an angle

    static const int ANGLE_CAP = 65536;
    static const int UP        = 0;
    static const int RIGHT     = 16384;
    static const int DOWN      = 32768;
    static const int LEFT      = 49152;

  public:
    Angle2D() : angle(0) { ; }
    Angle2D(const Angle2D & in_angle) : angle(in_angle.angle) { ; }
    Angle2D(double in_radians) : angle(radians * ANGLE_CAP / (2*PI)) { ; }
    Angle2D(int in_angle, bool okay) : angle(in_angle) { (void) okay; } // directly set internal value

    double AsRadians() const { return ((double) angle) * 2.0 * PI / ((double) ANGLE_CAP); }
    double AsDegrees() const { return ((double) angle) * 360.0 / ((double) ANGLE_CAP); }
    
    Angle2D & SetRadians(double in_radians) {
      angle = in_radians * ((double) ANGLE_CAP) / (2.0 * PI);
      return *this;
    }
    Angle2D & SetDegrees(double in_degrees) {
      angle = in_degrees * ((double) ANGLE_CAP) / 360.0;
      return *this;
    }
    Angle2D & PointUp()    { angle = UP;    return *this; }
    Angle2D & PointRight() { angle = RIGHT; return *this; }
    Angle2D & PointDown()  { angle = DOWN;  return *this; }
    Angle2D & PointLeft()  { angle = LEFT;  return *this; }

    // Chop off full circles
    Angle2D & Truncate() { angle &= (ANGLE_CAP - 1); return *this; }

    // Count full circles
    int CountFullCircles() { return angle >> 16; }

    // Some basic rotations...
    Angle2D & RotateRight() { angle += RIGHT; return *this; }
    Angle2D & RotateLeft()  { angle -= RIGHT; return *this; }
    Angle2D & RotateUTurn() { angle += DOWN; return *this; }
    Angle2D & RotateFull(int turns=1)  { angle += turns << 16; return *this; }

    Angle2D & RotateRadians(double in_radians) {
      angle += in_radians * ((double) ANGLE_CAP) / (2.0 * PI);
      return *this;
    }
    Angle2D & RotateDegrees(double in_degrees) {
      angle += in_degrees * ((double) ANGLE_CAP) / 360.0;
      return *this;
    }

    Angle2D & operator=(const Angle2D & _in) { angle = _in.angle; }
    
    bool operator==(const Angle2D & _in) const { return angle == _in.angle; }
    bool operator!=(const Angle2D & _in) const { return angle != _in.angle; }
    bool operator<(const Angle2D & _in)  const { return angle <  _in.angle; }
    bool operator<=(const Angle2D & _in) const { return angle <= _in.angle; }
    bool operator>(const Angle2D & _in)  const { return angle >  _in.angle; }
    bool operator>=(const Angle2D & _in) const { return angle >= _in.angle; }

    Angle2D operator+(const Angle2D & _in) const { return Angle(angle + _in.angle, true); }
    Angle2D operator-(const Angle2D & _in) const { return Angle(angle - _in.angle, true); }
    Angle2D operator*(double _in)          const { return Angle(angle * _in, true); }
    Angle2D operator*(int _in)             const { return Angle(angle * _in, true); }
    Angle2D operator/(double _in)          const { return Angle(angle / _in, true); }
    Angle2D operator/(int _in)             const { return Angle(angle / _in, true); }

    Angle2D & operator+=(const Angle2D & _in) { angle += _in.angle; return *this; }
    Angle2D & operator-=(const Angle2D & _in) { angle -= _in.angle; return *this; }
    Angle2D & operator*=(double _in)          { angle *= _in; return *this; }
    Angle2D & operator*=(int _in)             { angle *= _in; return *this; }
    Angle2D & operator/=(double _in)          { angle /= _in; return *this; }
    Angle2D & operator/=(int _in)             { angle /= _in; return *this; }

    double Sin() const { return sin(AsRadians()); } 
    double Cos() const { return cos(AsRadians()); } 
    double Tan() const { return tan(AsRadians()); } 
  }

};


#endif
