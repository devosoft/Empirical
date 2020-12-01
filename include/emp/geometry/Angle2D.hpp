//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  emp::Angle maintains an angle on a 2D surface.
//
//  The internal representation uses an int to represent angles.
//  First two bytes are number of full circles.
//  Last two bytes represent angle angles.
//

#ifndef EMP_ANGLE_2D_H
#define EMP_ANGLE_2D_H


#include <cmath>

#include "../math/constants.hpp"
#include "Point2D.hpp"

namespace emp {

  constexpr const int32_t sin_chart_1K[] =
  {    0,   25,   50,   75,  100,  125,  150,  175,  200,  224,  249,  273,  297,  321,  345,  369,
     392,  415,  438,  460,  483,  505,  526,  548,  569,  590,  610,  630,  650,  669,  688,  706,
     724,  742,  759,  775,  792,  807,  822,  837,  851,  865,  878,  891,  903,  915,  926,  936,
     946,  955,  964,  972,  980,  987,  993,  999, 1004, 1009, 1013, 1016, 1019, 1021, 1023, 1024,
    1024, 1024, 1023, 1021, 1019, 1016, 1013, 1009, 1004,  999,  993,  987,  980,  972,  964,  955,
     946,  936,  926,  915,  903,  891,  878,  865,  851,  837,  822,  807,  792,  775,  759,  742,
     724,  706,  688,  669,  650,  630,  610,  590,  569,  548,  526,  505,  483,  460,  438,  415,
     392,  369,  345,  321,  297,  273,  249,  224,  200,  175,  150,  125,  100,   75,   50,   25,
       0,  -24,  -49,  -74,  -99, -124, -149, -174, -199, -223, -248, -272, -296, -320, -344, -368,
    -391, -414, -437, -459, -482, -504, -525, -547, -568, -589, -609, -629, -649, -668, -687, -705,
    -723, -741, -758, -774, -791, -806, -821, -836, -850, -864, -877, -890, -902, -914, -925, -935,
    -945, -954, -963, -971, -979, -986, -992, -998,-1003,-1008,-1012,-1015,-1018,-1020,-1022,-1023,
   -1023,-1023,-1022,-1020,-1018,-1015,-1012,-1008,-1003, -998, -992, -986, -979, -971, -963, -954,
    -945, -935, -925, -914, -902, -890, -877, -864, -850, -836, -821, -806, -791, -774, -758, -741,
    -723, -705, -687, -668, -649, -629, -609, -589, -568, -547, -525, -504, -482, -459, -437, -414,
    -391, -368, -344, -320, -296, -272, -248, -223, -199, -174, -149, -124,  -99,  -74,  -49,  -24
  };
  constexpr const int32_t cos_chart_1K[] =
  { 1024, 1024, 1023, 1021, 1019, 1016, 1013, 1009, 1004,  999,  993,  987,  980,  972,  964,  955,
     946,  936,  926,  915,  903,  891,  878,  865,  851,  837,  822,  807,  792,  775,  759,  742,
     724,  706,  688,  669,  650,  630,  610,  590,  569,  548,  526,  505,  483,  460,  438,  415,
     392,  369,  345,  321,  297,  273,  249,  224,  200,  175,  150,  125,  100,   75,   50,   25,
       0,  -24,  -49,  -74,  -99, -124, -149, -174, -199, -223, -248, -272, -296, -320, -344, -368,
    -391, -414, -437, -459, -482, -504, -525, -547, -568, -589, -609, -629, -649, -668, -687, -705,
    -723, -741, -758, -774, -791, -806, -821, -836, -850, -864, -877, -890, -902, -914, -925, -935,
    -945, -954, -963, -971, -979, -986, -992, -998,-1003,-1008,-1012,-1015,-1018,-1020,-1022,-1023,
   -1023,-1023,-1022,-1020,-1018,-1015,-1012,-1008,-1003, -998, -992, -986, -979, -971, -963, -954,
    -945, -935, -925, -914, -902, -890, -877, -864, -850, -836, -821, -806, -791, -774, -758, -741,
    -723, -705, -687, -668, -649, -629, -609, -589, -568, -547, -525, -504, -482, -459, -437, -414,
    -391, -368, -344, -320, -296, -272, -248, -223, -199, -174, -149, -124,  -99,  -74,  -49,  -24,
       0,   25,   50,   75,  100,  125,  150,  175,  200,  224,  249,  273,  297,  321,  345,  369,
     392,  415,  438,  460,  483,  505,  526,  548,  569,  590,  610,  630,  650,  669,  688,  706,
     724,  742,  759,  775,  792,  807,  822,  837,  851,  865,  878,  891,  903,  915,  926,  936,
     946,  955,  964,  972,  980,  987,  993,  999, 1004, 1009, 1013, 1016, 1019, 1021, 1023, 1024
   };
  constexpr const int32_t tan_chart_1K[] =
  {    0,   25,   50,   76,  101,  126,  152,  178,  204,  230,  256,  283,  311,  338,  366,  395,
     424,  454,  484,  515,  547,  580,  614,  648,  684,  721,  759,  799,  840,  883,  928,  975,
    1024, 1076, 1130, 1187, 1248, 1312, 1381, 1454, 1533, 1617, 1708, 1808, 1916, 2034, 2165, 2310,
    2472, 2655, 2862, 3100, 3376, 3700, 4088, 4560, 5148, 5901, 6903, 8302,10397,13882,20844,41713,
   MIN_INT, -41712, -20843, -13881, -10396, -8301, -6902, -5900, -5147, -4559, -4087, -3699, -3375, -3099, -2861, -2654,
   -2471,-2309,-2164,-2033,-1915,-1807,-1707,-1616,-1532,-1453,-1380,-1311,-1247,-1186,-1129,-1075,
   -1023, -974, -927, -882, -839, -798, -758, -720, -683, -647, -613, -579, -546, -514, -483, -453,
    -423, -394, -365, -337, -310, -282, -255, -229, -203, -177, -151, -125, -100,  -75,  -49,  -24,
       0,   25,   50,   76,  101,  126,  152,  178,  204,  230,  256,  283,  311,  338,  366,  395,
     424,  454,  484,  515,  547,  580,  614,  648,  684,  721,  759,  799,  840,  883,  928,  975,
    1024, 1076, 1130, 1187, 1248, 1312, 1381, 1454, 1533, 1617, 1708, 1808, 1916, 2034, 2165, 2310,
    2472, 2655, 2862, 3100, 3376, 3700, 4088, 4560, 5148, 5901, 6903, 8302,10397,13882,20844,41713,
   MIN_INT, -41712, -20843, -13881, -10396, -8301, -6902, -5900, -5147, -4559, -4087, -3699, -3375, -3099, -2861, -2654,
   -2471,-2309,-2164,-2033,-1915,-1807,-1707,-1616,-1532,-1453,-1380,-1311,-1247,-1186,-1129,-1075,
   -1023, -974, -927, -882, -839, -798, -758, -720, -683, -647, -613, -579, -546, -514, -483, -453,
    -423, -394, -365, -337, -310, -282, -255, -229, -203, -177, -151, -125, -100,  -75,  -49,  -24,
  };

  class Angle {
  private:
    uint32_t angle;    // Int representation of an angle

    static constexpr double ANGLE_CAP  = 65536.0;
    static constexpr uint32_t UP       = 0;
    static constexpr uint32_t RIGHT    = 16384;
    static constexpr uint32_t DOWN     = 32768;
    static constexpr uint32_t LEFT     = 49152;

  public:
    constexpr Angle() : angle(0) { ; }
    constexpr Angle(const Angle & in_angle) : angle(in_angle.angle) { ; }
    constexpr Angle(double radians) : angle((uint32_t)(radians * ANGLE_CAP / (2.0*PI))) { ; }
    constexpr Angle(uint32_t in_angle, bool) : angle(in_angle) { ; } // directly set internal value

    constexpr double AsPortion() const { return ((double) (angle % 0xFFFF)) / ANGLE_CAP; }
    constexpr double AsRadians() const { return ((double) angle) * 2.0 * PI / ANGLE_CAP; }
    constexpr double AsDegrees() const { return ((double) angle) * 360.0 / ANGLE_CAP; }

    Angle & SetPortion(double portion) {
      angle = (uint32_t) (portion * ANGLE_CAP);
      return *this;
    }
    Angle & SetRadians(double radians) {
      angle = (uint32_t) (radians * ANGLE_CAP / (2.0 * PI));
      return *this;
    }
    Angle & SetDegrees(double degrees) {
      angle = (uint32_t) (degrees * ANGLE_CAP / 360.0);
      return *this;
    }
    Angle & PointUp()    { angle = UP;    return *this; }
    Angle & PointRight() { angle = RIGHT; return *this; }
    Angle & PointDown()  { angle = DOWN;  return *this; }
    Angle & PointLeft()  { angle = LEFT;  return *this; }

    // Chop off full circles
    Angle & Truncate() { angle &= 0xFFFF; return *this; }

    // Count full circles
    uint32_t CountFullCircles() { return angle >> 16; }

    // Some basic rotations...
    Angle & RotateRight() { angle += RIGHT; return *this; }
    Angle & RotateLeft()  { angle -= RIGHT; return *this; }
    Angle & RotateUTurn() { angle += DOWN; return *this; }
    Angle & RotateFull(uint32_t turns=1)  { angle += turns << 16; return *this; }

    Angle & RotateRadians(double radians) {
      angle += (uint32_t) (radians * ANGLE_CAP / (2.0 * PI));
      return *this;
    }
    Angle & RotateDegrees(double degrees) {
      if (degrees < 0.0) degrees += 360.0;
      emp_assert(degrees >= 0.0);  // Don't modify by less than -360 (@CAO Fix?)
      angle += (uint32_t) (degrees * ANGLE_CAP / 360.0);
      return *this;
    }

    Angle & operator=(const Angle & _in) { angle = _in.angle; return *this; }

    constexpr bool operator==(const Angle & _in) const { return angle == _in.angle; }
    constexpr bool operator!=(const Angle & _in) const { return angle != _in.angle; }
    constexpr bool operator<(const Angle & _in)  const { return angle <  _in.angle; }
    constexpr bool operator<=(const Angle & _in) const { return angle <= _in.angle; }
    constexpr bool operator>(const Angle & _in)  const { return angle >  _in.angle; }
    constexpr bool operator>=(const Angle & _in) const { return angle >= _in.angle; }

    constexpr Angle operator+(const Angle & _in) const { return Angle(angle + _in.angle, true); }
    constexpr Angle operator-(const Angle & _in) const { return Angle(angle - _in.angle, true); }

    constexpr Angle operator*(double _in)  const { return Angle((uint32_t) (angle * _in), true); }
    constexpr Angle operator/(double _in)  const { return Angle((uint32_t) (angle / _in), true); }
    // constexpr Angle operator*(int _in)     const { return Angle(angle * _in, true); }
    // constexpr Angle operator/(int _in)     const { return Angle(angle / _in, true); }

    Angle & operator+=(const Angle & _in) { angle += _in.angle; return *this; }
    Angle & operator-=(const Angle & _in) { angle -= _in.angle; return *this; }
    Angle & operator*=(double _in)        { angle = (uint32_t) (angle * _in); return *this; }
    Angle & operator/=(double _in)        { angle = (uint32_t) (angle / _in); return *this; }

    double Sin() const { return sin(AsRadians()); }
    double Cos() const { return cos(AsRadians()); }
    double Tan() const { return tan(AsRadians()); }

    // Quicker version of sin/cos/tan -- these are low precision and return an int: result * 1024
    constexpr int Sin_Quick1K() const { return emp::sin_chart_1K[ (angle >> 8) & 255 ]; }
    constexpr int Cos_Quick1K() const { return emp::cos_chart_1K[ (angle >> 8) & 255 ]; }
    constexpr int Tan_Quick1K() const { return emp::tan_chart_1K[ (angle >> 8) & 255 ]; }

    Point GetPoint(double distance=1.0) const {
      return Point(Sin() * distance, Cos() * distance);
    }
    Point GetPoint(const Point & start_point, double distance=1.0) const {
      return start_point.GetOffset(Sin() * distance, Cos() * distance);
    }
  };

}


#endif
