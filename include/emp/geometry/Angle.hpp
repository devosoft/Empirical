/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/geometry/Angle.hpp
 * @brief emp::Angle maintains a geometric angle.
 *
 * The internal representation uses an uint16_t to represent angles in a 2 byte system.
 * I.e., internally, all angles go from 0 to 65,535
 */

#pragma once

#ifndef INCLUDE_EMP_GEOMETRY_ANGLE_HPP_GUARD
#define INCLUDE_EMP_GEOMETRY_ANGLE_HPP_GUARD

#include <cmath>
#include <compare>
#include <cstdint>

#include "../math/constants.hpp"

#include "Point2D.hpp"

namespace emp {

  // 256 sine values over one period, from -1023 to +1024
  constexpr const int32_t sin_chart_1K[] =
    {0,     25,    50,    75,    100,   125,   150,   175,   200,   224,   249,   273,   297,
     321,   345,   369,   392,   415,   438,   460,   483,   505,   526,   548,   569,   590,
     610,   630,   650,   669,   688,   706,   724,   742,   759,   775,   792,   807,   822,
     837,   851,   865,   878,   891,   903,   915,   926,   936,   946,   955,   964,   972,
     980,   987,   993,   999,   1004,  1009,  1013,  1016,  1019,  1021,  1023,  1024,  1024,
     1024,  1023,  1021,  1019,  1016,  1013,  1009,  1004,  999,   993,   987,   980,   972,
     964,   955,   946,   936,   926,   915,   903,   891,   878,   865,   851,   837,   822,
     807,   792,   775,   759,   742,   724,   706,   688,   669,   650,   630,   610,   590,
     569,   548,   526,   505,   483,   460,   438,   415,   392,   369,   345,   321,   297,
     273,   249,   224,   200,   175,   150,   125,   100,   75,    50,    25,    0,     -24,
     -49,   -74,   -99,   -124,  -149,  -174,  -199,  -223,  -248,  -272,  -296,  -320,  -344,
     -368,  -391,  -414,  -437,  -459,  -482,  -504,  -525,  -547,  -568,  -589,  -609,  -629,
     -649,  -668,  -687,  -705,  -723,  -741,  -758,  -774,  -791,  -806,  -821,  -836,  -850,
     -864,  -877,  -890,  -902,  -914,  -925,  -935,  -945,  -954,  -963,  -971,  -979,  -986,
     -992,  -998,  -1003, -1008, -1012, -1015, -1018, -1020, -1022, -1023, -1023, -1023, -1022,
     -1020, -1018, -1015, -1012, -1008, -1003, -998,  -992,  -986,  -979,  -971,  -963,  -954,
     -945,  -935,  -925,  -914,  -902,  -890,  -877,  -864,  -850,  -836,  -821,  -806,  -791,
     -774,  -758,  -741,  -723,  -705,  -687,  -668,  -649,  -629,  -609,  -589,  -568,  -547,
     -525,  -504,  -482,  -459,  -437,  -414,  -391,  -368,  -344,  -320,  -296,  -272,  -248,
     -223,  -199,  -174,  -149,  -124,  -99,   -74,   -49,   -24};

  // 256 cosine values over one period, from -1023 to +1024
  constexpr const int32_t cos_chart_1K[] =
    {1024,  1024,  1023,  1021,  1019,  1016,  1013,  1009,  1004,  999,   993,   987,   980,
     972,   964,   955,   946,   936,   926,   915,   903,   891,   878,   865,   851,   837,
     822,   807,   792,   775,   759,   742,   724,   706,   688,   669,   650,   630,   610,
     590,   569,   548,   526,   505,   483,   460,   438,   415,   392,   369,   345,   321,
     297,   273,   249,   224,   200,   175,   150,   125,   100,   75,    50,    25,    0,
     -24,   -49,   -74,   -99,   -124,  -149,  -174,  -199,  -223,  -248,  -272,  -296,  -320,
     -344,  -368,  -391,  -414,  -437,  -459,  -482,  -504,  -525,  -547,  -568,  -589,  -609,
     -629,  -649,  -668,  -687,  -705,  -723,  -741,  -758,  -774,  -791,  -806,  -821,  -836,
     -850,  -864,  -877,  -890,  -902,  -914,  -925,  -935,  -945,  -954,  -963,  -971,  -979,
     -986,  -992,  -998,  -1003, -1008, -1012, -1015, -1018, -1020, -1022, -1023, -1023, -1023,
     -1022, -1020, -1018, -1015, -1012, -1008, -1003, -998,  -992,  -986,  -979,  -971,  -963,
     -954,  -945,  -935,  -925,  -914,  -902,  -890,  -877,  -864,  -850,  -836,  -821,  -806,
     -791,  -774,  -758,  -741,  -723,  -705,  -687,  -668,  -649,  -629,  -609,  -589,  -568,
     -547,  -525,  -504,  -482,  -459,  -437,  -414,  -391,  -368,  -344,  -320,  -296,  -272,
     -248,  -223,  -199,  -174,  -149,  -124,  -99,   -74,   -49,   -24,   0,     25,    50,
     75,    100,   125,   150,   175,   200,   224,   249,   273,   297,   321,   345,   369,
     392,   415,   438,   460,   483,   505,   526,   548,   569,   590,   610,   630,   650,
     669,   688,   706,   724,   742,   759,   775,   792,   807,   822,   837,   851,   865,
     878,   891,   903,   915,   926,   936,   946,   955,   964,   972,   980,   987,   993,
     999,   1004,  1009,  1013,  1016,  1019,  1021,  1023,  1024};

  // 256 tangent values over one period, scaled based on charts above.
  constexpr const int32_t tan_chart_1K[] = {
    0,       25,     50,     76,     101,     126,    152,    178,    204,    230,   256,   283,
    311,     338,    366,    395,    424,     454,    484,    515,    547,    580,   614,   648,
    684,     721,    759,    799,    840,     883,    928,    975,    1024,   1076,  1130,  1187,
    1248,    1312,   1381,   1454,   1533,    1617,   1708,   1808,   1916,   2034,  2165,  2310,
    2472,    2655,   2862,   3100,   3376,    3700,   4088,   4560,   5148,   5901,  6903,  8302,
    10397,   13882,  20844,  41713,  MIN_INT, -41712, -20843, -13881, -10396, -8301, -6902, -5900,
    -5147,   -4559,  -4087,  -3699,  -3375,   -3099,  -2861,  -2654,  -2471,  -2309, -2164, -2033,
    -1915,   -1807,  -1707,  -1616,  -1532,   -1453,  -1380,  -1311,  -1247,  -1186, -1129, -1075,
    -1023,   -974,   -927,   -882,   -839,    -798,   -758,   -720,   -683,   -647,  -613,  -579,
    -546,    -514,   -483,   -453,   -423,    -394,   -365,   -337,   -310,   -282,  -255,  -229,
    -203,    -177,   -151,   -125,   -100,    -75,    -49,    -24,    0,      25,    50,    76,
    101,     126,    152,    178,    204,     230,    256,    283,    311,    338,   366,   395,
    424,     454,    484,    515,    547,     580,    614,    648,    684,    721,   759,   799,
    840,     883,    928,    975,    1024,    1076,   1130,   1187,   1248,   1312,  1381,  1454,
    1533,    1617,   1708,   1808,   1916,    2034,   2165,   2310,   2472,   2655,  2862,  3100,
    3376,    3700,   4088,   4560,   5148,    5901,   6903,   8302,   10397,  13882, 20844, 41713,
    MIN_INT, -41712, -20843, -13881, -10396,  -8301,  -6902,  -5900,  -5147,  -4559, -4087, -3699,
    -3375,   -3099,  -2861,  -2654,  -2471,   -2309,  -2164,  -2033,  -1915,  -1807, -1707, -1616,
    -1532,   -1453,  -1380,  -1311,  -1247,   -1186,  -1129,  -1075,  -1023,  -974,  -927,  -882,
    -839,    -798,   -758,   -720,   -683,    -647,   -613,   -579,   -546,   -514,  -483,  -453,
    -423,    -394,   -365,   -337,   -310,    -282,   -255,   -229,   -203,   -177,  -151,  -125,
    -100,    -75,    -49,    -24,
  };

  class Angle {
  private:
    uint16_t angle = 0;       // Int representation of an angle using 2byte system.

    static constexpr double ANGLE_CAP     = 65536.0;
    static constexpr uint16_t ANGLE_UP    = 0;
    static constexpr uint16_t ANGLE_RIGHT = 16384;
    static constexpr uint16_t ANGLE_DOWN  = 32768;
    static constexpr uint16_t ANGLE_LEFT  = 49152;

    static constexpr double MAX_RADIANS = 2.0 * PI;
    static constexpr double MAX_DEGREES = 360.0;

    static constexpr double BYTE2_TO_DEGREE  = MAX_DEGREES / ANGLE_CAP;
    static constexpr double BYTE2_TO_PORTION = 1.0 / ANGLE_CAP;
    static constexpr double BYTE2_TO_RADIANS = MAX_RADIANS / ANGLE_CAP;

    static constexpr double DEGREES_TO_BYTE2 = ANGLE_CAP / MAX_DEGREES;
    static constexpr double DEGREES_TO_PORTION = 1.0 / MAX_DEGREES;
    static constexpr double DEGREES_TO_RADIANS = MAX_RADIANS / MAX_DEGREES;

    static constexpr double RADIANS_TO_BYTE2 = ANGLE_CAP / MAX_RADIANS;
    static constexpr double RADIANS_TO_DEGREE = MAX_DEGREES / MAX_RADIANS;
    static constexpr double RADIANS_TO_PORTION = 1.0 / MAX_RADIANS;

    static constexpr uint16_t FromRadians(double rads) {
      return static_cast<uint16_t>(rads * RADIANS_TO_BYTE2);
    }

    constexpr Angle(uint16_t in_angle, bool) : angle(in_angle) {}  // directly set internal value

  public:
    constexpr Angle()              = default;
    constexpr Angle(const Angle &) = default;

    constexpr Angle(double radians) : angle(FromRadians(radians)) {}

    constexpr Angle & operator=(const Angle &) = default;

    [[nodiscard]] constexpr auto operator<=>(const Angle &) const = default;

    // A series of conversion helpers for different methods of tracking angles.
    static constexpr double Byte2ToDegree(uint16_t byte2) { return byte2 * BYTE2_TO_DEGREE; }
    static constexpr double Byte2ToPortion(uint16_t byte2) { return byte2 * BYTE2_TO_PORTION; }
    static constexpr double Byte2ToRadians(uint16_t byte2) { return byte2 * BYTE2_TO_RADIANS; }

    static constexpr uint16_t DegreesToByte2(double degrees) {
      return static_cast<uint16_t>(degrees * DEGREES_TO_BYTE2);
    }
    static constexpr double DegreesToPortion(double degrees) {
      return degrees * DEGREES_TO_PORTION;
    }
    static constexpr double DegreesToRadians(double degrees) {
      return degrees * DEGREES_TO_RADIANS;
    }

    static constexpr uint16_t PortionToByte2(double portion) {
      return static_cast<uint16_t>(portion * ANGLE_CAP);
    }
    static constexpr double PortionToDegree(double portion) { return portion * MAX_DEGREES; }
    static constexpr double PortionToRadians(double portion) { return portion * MAX_RADIANS; }

    static constexpr uint16_t RadiansToByte2(double rads) {
      return static_cast<uint16_t>(rads * RADIANS_TO_BYTE2);
    }
    static constexpr double RadiansToDegree(double rads) { return rads * RADIANS_TO_DEGREE; }
    static constexpr double RadiansToPortion(double rads) { return rads * RADIANS_TO_PORTION; }

    [[nodiscard]] constexpr double AsPortion() const { return Byte2ToPortion(angle); }
    [[nodiscard]] constexpr double AsRadians() const { return Byte2ToRadians(angle); }
    [[nodiscard]] constexpr double AsDegrees() const { return Byte2ToDegree(angle); }

    constexpr Angle & SetPortion(double portion) {
      angle = PortionToByte2(portion);
      return *this;
    }

    constexpr Angle & SetRadians(double radians) {
      angle = RadiansToByte2(radians);
      return *this;
    }

    constexpr Angle & SetDegrees(double degrees) {
      angle = DegreesToByte2(degrees);
      return *this;
    }

    static constexpr Angle UP() { return { ANGLE_UP, true}; }
    static constexpr Angle RIGHT() { return { ANGLE_RIGHT, true}; }
    static constexpr Angle DOWN() { return { ANGLE_DOWN, true}; }
    static constexpr Angle LEFT() { return { ANGLE_LEFT, true}; }

    // Some basic rotations...
    constexpr Angle & RotateRight() {
      angle += ANGLE_RIGHT;
      return *this;
    }

    constexpr Angle & RotateLeft() {
      angle += ANGLE_LEFT;
      return *this;
    }

    constexpr Angle & RotateUTurn() {
      angle += ANGLE_DOWN;
      return *this;
    }

    constexpr Angle & RotateDegrees(double degrees) {
      angle += RadiansToByte2(degrees);
      return *this;
    }

    constexpr Angle & RotatePortion(double portion) {
      angle += PortionToByte2(portion);
      return *this;
    }

    constexpr Angle & RotateRadians(double radians) {
      angle += RadiansToByte2(radians);
      return *this;
    }

    [[nodiscard]] constexpr Angle operator+(const Angle & in) const {
      return {static_cast<uint16_t>(angle + in.angle), true };
    }

    [[nodiscard]] constexpr Angle operator-(const Angle & in) const {
      return {static_cast<uint16_t>(angle - in.angle), true };
    }

    [[nodiscard]] constexpr Angle operator*(double in) const {
      return {static_cast<uint16_t>(angle * in), true};
    }

    [[nodiscard]] constexpr Angle operator/(double in) const {
      return {static_cast<uint16_t>(angle / in), true};
    }

    constexpr Angle & operator+=(const Angle & _in) {
      angle += _in.angle;
      return *this;
    }

    constexpr Angle & operator-=(const Angle & _in) {
      angle -= _in.angle;
      return *this;
    }

    constexpr Angle & operator*=(double _in) {
      angle = static_cast<uint16_t>(angle * _in);
      return *this;
    }

    constexpr Angle & operator/=(double _in) {
      angle = static_cast<uint16_t>(angle / _in);
      return *this;
    }

    [[nodiscard]] double Sin() const { return sin(AsRadians()); }
    [[nodiscard]] double Cos() const { return cos(AsRadians()); }
    [[nodiscard]] double Tan() const { return tan(AsRadians()); }

    // Quicker version of sin/cos/tan -- these are low precision and return an int: result * 1024
    [[nodiscard]] constexpr int QuickSin() const { return emp::sin_chart_1K[angle >> 8]; }
    [[nodiscard]] constexpr int QuickCos() const { return emp::cos_chart_1K[angle >> 8]; }
    [[nodiscard]] constexpr int QuickTan() const { return emp::tan_chart_1K[angle >> 8]; }

    [[nodiscard]] constexpr Point GetPoint(double distance = 1.0) const {
      return Point{Sin() * distance, Cos() * distance};
    }

    /// @brief Overload ostream operator to return Print.
    friend std::ostream & operator<<(std::ostream & out, Angle angle) {
      out << angle.angle;
      return out;
    }
  };

}  // namespace emp


#endif  // #ifndef INCLUDE_EMP_GEOMETRY_ANGLE_HPP_GUARD
