/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Color.hpp
 * @brief Color objects
 * 
 * DEVELOPER NOTES:
 * - Change to storing RGB (and maybe alpha?)
 * - Build a chart for converting color names.
 * - Add a ToANSI option on colors (where it aims for the nearest match?)
 * - Some kind of measure of color distance?
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_COLOR_HPP_GUARD
#define INCLUDE_EMP_WEB_COLOR_HPP_GUARD

#include "../tools/String.hpp"

namespace emp {

  class Color {
  protected:
    std::string m_color="";
  public:
    constexpr Color(std::string name="") : m_color(name) {}
    constexpr Color(const Color &) = default;
    constexpr Color(Color &&) = default;
    constexpr Color & operator=(const Color &) = default;
    constexpr Color & operator=(Color &&) = default;
    constexpr Color & operator=(std::string name) {
      m_color = name;
      return *this;
    };

    constexpr bool IsSet() const { return m_color.size() > 0; }
    constexpr operator bool() const { return IsSet(); }

    const std::string & ToString() const { return m_color; }
    constexpr const char * c_str() const { return m_color.c_str(); }
  };

  namespace Palette {
    static constexpr Color NONE{""};
    
    static constexpr Color BLACK{"black"};
    static constexpr Color RED{"red"};
    static constexpr Color GREEN{"green"};
    static constexpr Color BLUE{"blue"};
    static constexpr Color CYAN{"cyan"};
    static constexpr Color MAGENTA{"magenta"};
    static constexpr Color YELLOW{"yellow"};
    static constexpr Color WHITE{"white"};
    static constexpr Color GRAY{"gray"};
    static constexpr Color PINK{"pink"};
  }

  class HSL : public Color {
  public:
    HSL(double h, double s, double l) : Color(MakeString("hsl(", h, ',', s, "%,", l, "%)")) {
      emp_assert(h >= 0 && h <= 360, h);
      emp_assert(s >= 0 && s <= 100, s);
      emp_assert(l >= 0 && l <= 100, l);
    }
  };

  class RGB : public Color {
  public:
    RGB(int r, int g, int b) : Color("#" + MakeHexString(r, 2).AppendHex(g, 2).AppendHex(b, 2)) {
      emp_assert(r >= 0 && r <= 255);
      emp_assert(g >= 0 && g <= 255);
      emp_assert(b >= 0 && b <= 255);
    }

    /// With an alpha channel
    RGB(int r, int g, int b, double a) : Color(MakeString("rgba(", r, ',', g, ',', b, ',', a, ')')){
      emp_assert(r >= 0 && r <= 255);
      emp_assert(g >= 0 && g <= 255);
      emp_assert(b >= 0 && b <= 255);
      emp_assert(a >= 0 && a <= 1.0);
    }
  };

  /// Generate a string to describe a JS color out of HSV values.
  class HSV : public HSL {
  private:
    static double HSVtoS([[maybe_unused]] double h, double s, double v) {
      double l = v * (1 - s / 2.0);
      if (l == 0.0 || l == 1.0) return 0.0;
      return (v - l) / std::min(l, 1.0 - l);
    }
  public:
    HSV(double h, double s, double v) : HSL(h, HSVtoS(h, s, v), v * (1 - s / 2.0)) {
      emp_assert(h >= 0.0 && h <= 360.0);
      emp_assert(s >= 0.0 && s <= 1.0);
      emp_assert(v >= 0.0 && v <= 1.0);
    }
  };

  /// Generate a vector of colors with a range of hues, and fixed saturation and luminosity,
  emp::vector<Color> MakeHueRange(size_t range_size,
                                double min_h = 0.0,
                                double max_h = 360.0,
                                double s        = 100,
                                double l        = 50) {
    emp::vector<Color> result;
    result.reserve(range_size);

    double step_size = (max_h - min_h) / (double) range_size;
    for (size_t i = 0; i < range_size; ++i) {
      double h   = min_h + step_size * i + step_size/2.0;
      result.emplace_back(HSL{h, s, l});
    }

    return result;
  }

  /// Generate a vector of color strings providing ranges of all of hue, saturation and luminosity.
  emp::vector<Color> MakeHSLRange(size_t range_size,
                                double min_h = 0.0,
                                double max_h = 360.0,
                                double min_s = 100.0,
                                double max_s = 100.0,
                                double min_l = 50.0,
                                double max_l = 50.0) {
    emp::vector<Color> result;
    result.reserve(range_size);

    double h_step = (max_h - min_h) / range_size;
    double s_step = (max_s - min_s) / range_size;
    double l_step = (max_l - min_l) / range_size;

    for (size_t i = 0; i < range_size; ++i) {
      double h = min_h + h_step * i + h_step/2.0;
      double s = min_s + s_step * i + s_step/2.0;
      double l = min_l + l_step * i + l_step/2.0;
      if (h > 360.0) { h -= 360.0; }
      if (s > 100.0) { s -= 100.0; }
      if (l > 100.0) { l -= 100.0; }
      result.emplace_back(HSL{h, s, l});
    }

    return result;
  }


}  // namespace emp


#endif  // #ifndef INCLUDE_EMP_WEB_COLOR_HPP_GUARD
