/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  color_map.hpp
 *  @brief Tools to dynamically build (and cache) color maps.
 */

#ifndef EMP_COLOR_MAP_H
#define EMP_COLOR_MAP_H

#include <cmath>
#include <iomanip>
#include <map>
#include <string>
#include <tuple>

#include "../base/vector.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  namespace {
    using dHueMapKey = std::tuple<int, double, double, int, int>;
    using dHueMap = std::map<dHueMapKey, emp::vector<std::string> >;

    dHueMap hue_maps;
  }

  /// Generate a string to describe a JS color out of HSL values.
  std::string ColorHSL(double h, double s, double l) {
    emp_assert(h >= 0 && h <= 360, h);
    emp_assert(s >= 0 && s <= 100, s);
    emp_assert(l >= 0 && l <= 100, l);
    std::stringstream ss;
    ss << "hsl(" <<  h << ',' << s << "%," <<  l << "%)";
    return ss.str();
  }

  /// Generate a string to describe a JS color out of RGB values.
  std::string ColorRGB(int r, int g, int b) {
    emp_assert(r >= 0 && r <= 255);
    emp_assert(g >= 0 && g <= 255);
    emp_assert(b >= 0 && b <= 255);
    std::stringstream ss;
    ss << '#' << std::setw(2) << std::setfill('0') << std::hex << r
       << std::setw(2) << std::setfill('0') << std::hex << g
       << std::setw(2) << std::setfill('0') << std::hex << b;
    return ss.str();
  }

  /// Generate a string to describe a JS color with an alpha channel.
  std::string ColorRGB(int r, int g, int b, double a) {
    emp_assert(r >= 0 && r <= 255);
    emp_assert(g >= 0 && g <= 255);
    emp_assert(b >= 0 && b <= 255);
    emp_assert(a >= 0 && a <= 1.0);
    std::stringstream ss;
    ss << "rgba(" << r << ',' << g << ',' << b << ',' << a << ')';
    return ss.str();
  }

  /// Generate a string to describe a JS color out of HSV values.
  std::string ColorHSV(double h, double s, double v) {
    // adapted from https://gist.github.com/kuathadianto/200148f53616cbd226d993b400214a7f

    emp_assert( h >= 0.0 && h <= 360.0);
    emp_assert( s >= 0.0 && s <= 1.0);
    emp_assert( v >= 0.0 && v <= 1.0);

  	double c = s * v;
  	double x = c * (1 - std::abs(std::fmod(h / 60.0, 2) - 1));
  	double m = v - c;
  	double rs, gs, bs;

  	if(h >= 0 && h < 60) {
  		rs = c;
  		gs = x;
  		bs = 0;
  	} else if(h >= 60 && h < 120) {
  		rs = x;
  		gs = c;
  		bs = 0;
  	} else if(h >= 120 && h < 180) {
  		rs = 0;
  		gs = c;
  		bs = x;
  	} else if(h >= 180 && h < 240) {
  		rs = 0;
  		gs = x;
  		bs = c;
  	} else if(h >= 240 && h < 300) {
  		rs = x;
  		gs = 0;
  		bs = c;
  	} else {
  		rs = c;
  		gs = 0;
  		bs = x;
  	}

    int r = (int) ((rs + m) * 255);
    int g = (int) ((gs + m) * 255);
    int b = (int) ((bs + m) * 255);

    return ColorRGB(r, g, b);
  }

  /// Generate a vector of color strings with a specified range of hues, and fixed satuation and
  /// luminosity,
  const emp::vector<std::string> &
  GetHueMap(size_t map_size, double min_h=0.0, double max_h=360.0, int s=100, int l=50) {
    dHueMapKey map_key = std::make_tuple(map_size, min_h, max_h, s, l);

    // Grab the current map out of the cache.
    emp::vector<std::string> & cur_map = hue_maps[map_key];

    // If we've already asked for an identical map before, skip map generation!
    if (cur_map.size() != (std::size_t) map_size) {

      // Otherwise generate this map...
      cur_map.resize(map_size);
      double step_size = (max_h - min_h) / (double) map_size;
      for (size_t i = 0; i < map_size; ++i) {
        double h = min_h + step_size * i;
        cur_map[i] = ColorHSL(h, s, l);
      }
    }

    return cur_map;
  }

  /// Generate a vector of color strings providing ranges of all of hue, satuation and luminosity.
  emp::vector<std::string>
  GetHSLMap(size_t map_size, double min_h=0.0, double max_h=360.0,
            int min_s=100, int max_s=100,
            int min_l=50, int max_l=50) {

    // @CAO: Should cache maps!
    emp::vector<std::string> cur_map(map_size);
    double h_step = (max_h - min_h) / (double) map_size;
    double s_step = (max_s - min_s) / (double) map_size;
    double l_step = (max_l - min_l) / (double) map_size;
    for (size_t i = 0; i < map_size; ++i) {
      double h = min_h + h_step * i;
      double s = min_s + s_step * i;
      double l = min_l + l_step * i;
      if (h > 360) h -= 360;
      if (s > 100) s -= 100;
      if (l > 100) l -= 100;
      cur_map[i] = ColorHSL(h, s, l);
    }

    return cur_map;
  }


}


#endif
