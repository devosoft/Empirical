#ifndef EMP_COLOR_MAP_H
#define EMP_COLOR_MAP_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to dynamically build (and cache) color maps.
//

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "../tools/string_utils.h"

namespace emp {

  namespace internal {
    using dHueMapKey = std::tuple<int, double, double, int, int>;
    using dHueMap = std::map<dHueMapKey, std::vector<std::string> >;

    dHueMap hue_maps;
  }
  
  const std::vector<std::string> &
  GetHueMap(int map_size, double min_h=0.0, double max_h=360.0, int s=100, int l=50) {
    internal::dHueMapKey map_key = std::make_tuple(map_size, min_h, max_h, s, l);

    // Grab the current map out of the cache.
    std::vector<std::string> & cur_map = internal::hue_maps[map_key];

    // If we've already asked for an identical map before, skip map generation!
    if (cur_map.size() != map_size) {
      
      // Otherwise generate this map...
      cur_map.resize(map_size);
      double step_size = (max_h - min_h) / (double) map_size;
      for (int i = 0; i < map_size; ++i) {
        double cur_hue = min_h + step_size * i;
        cur_map[i] = emp::to_string("hsl(", cur_hue, ",", s, "%,", l, "%)");
      }
    }

    return cur_map;
  }


}


#endif
