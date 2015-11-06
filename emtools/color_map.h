// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_COLOR_MAP_H
#define EMP_COLOR_MAP_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to dynamically build (and cache) color maps.
//

#include <iomanip>
#include <map>
#include <string>
#include <tuple>

#include "../tools/string_utils.h"
#include "../tools/vector.h"

namespace emp {

  namespace internal {
    using dHueMapKey = std::tuple<int, double, double, int, int>;
    using dHueMap = std::map<dHueMapKey, emp::vector<std::string> >;

    dHueMap hue_maps;
  }

  // Generate a string to describe a JS color out of HSL values.
  std::string ColorHSL(double h, double s, double l) {
    emp_assert(h >= 0 && h <= 360);
    emp_assert(s >= 0 && s <= 100);
    emp_assert(l >= 0 && l <= 100);
    std::stringstream ss;
    ss << "hsl(" <<  h << ',' << s << "%," <<  l << "%)";
    return ss.str();
  }

  // Generate a string to describe a JS color out of RGB values.
  std::string ColorRGB(int r, int g, int b) {
    emp_assert(r >= 0 && r < 255);
    emp_assert(g >= 0 && g < 255);
    emp_assert(b >= 0 && b < 255);
    std::stringstream ss;
    ss << '#' << std::setw(2) << std::setfill('0') << std::hex << r
       << std::setw(2) << std::setfill('0') << std::hex << g
       << std::setw(2) << std::setfill('0') << std::hex << b;
    return ss.str();
  }

  // Generate a string to describe a JS color with an alpha channel.
  std::string ColorRGB(int r, int g, int b, double a) {
    emp_assert(r >= 0 && r < 255);
    emp_assert(g >= 0 && g < 255);
    emp_assert(b >= 0 && b < 255);
    std::stringstream ss;
    ss << "rgba(" << r << ',' << g << ',' << b << ',' << a << ')';    
    return ss.str();
  }

  const emp::vector<std::string> &
  GetHueMap(int map_size, double min_h=0.0, double max_h=360.0, int s=100, int l=50) {
    internal::dHueMapKey map_key = std::make_tuple(map_size, min_h, max_h, s, l);

    // Grab the current map out of the cache.
    emp::vector<std::string> & cur_map = internal::hue_maps[map_key];

    // If we've already asked for an identical map before, skip map generation!
    if (cur_map.size() != map_size) {
      
      // Otherwise generate this map...
      cur_map.resize(map_size);
      double step_size = (max_h - min_h) / (double) map_size;
      for (int i = 0; i < map_size; ++i) {
        double h = min_h + step_size * i;
        cur_map[i] = ColorHSL(h, s, l);
      }
    }

    return cur_map;
  }


}


#endif
