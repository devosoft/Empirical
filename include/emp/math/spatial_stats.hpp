/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019
 *
 *  @file spatial_stats.hpp
 *  @brief Functions for calculating various spatial statistics.
 *  @note Status: BETA
 */

#ifndef EMP_MATH_SPATIAL_STATS_HPP_INCLUDE
#define EMP_MATH_SPATIAL_STATS_HPP_INCLUDE

#include <algorithm>

#include "../base/vector.hpp"
#include "../Evolve/World.hpp"

#include "stats.hpp"

namespace emp {


  // Point on grid stats

  template <typename ORG>
  double GridPointDensity(emp::World<ORG> & w, size_t xid, size_t yid, int neighborhood_size = 2) {
    double density = 0;
    int x_modifier = 0;
    int y_modifier = 0;

    if ((int)xid < neighborhood_size) {
      x_modifier = neighborhood_size - xid;
    } else if ((int)xid + neighborhood_size >= (int)w.GetWidth()) {
      x_modifier = xid + neighborhood_size - w.GetWidth() + 1;
    }

    if ((int)yid < neighborhood_size) {
      y_modifier = neighborhood_size - yid;
    } else if ((int)yid + neighborhood_size >= (int)w.GetHeight()) {
      y_modifier = yid + neighborhood_size - w.GetHeight() + 1;
    }

    double denominator = (neighborhood_size*2 + 1 - x_modifier) * (neighborhood_size * 2 + 1 - y_modifier);

    for (size_t x = std::max((int)xid - neighborhood_size, 0); (int)x <= std::min((int)xid + neighborhood_size, (int)w.GetWidth()-1); x++) {
      for (size_t y = std::max((int)yid - neighborhood_size, 0); (int)y <= std::min((int)yid + neighborhood_size, (int)w.GetHeight()-1); y++) {
        if (w.IsOccupied(x+y*w.GetWidth())) {
          density++;
        }
      }
    }

    return density/denominator;
  }

  template <typename ORG>
  double GridPointShannonEntropy(emp::World<ORG> & w, size_t xid, size_t yid, int neighborhood_size = 2) {
    // double density = 0;
    // int x_modifier = 0;
    // int y_modifier = 0;
    // if ((int)xid < neighborhood_size) {
    //   x_modifier = neighborhood_size - xid;
    // } else if (xid + neighborhood_size >= w.GetWidth()) {
    //   x_modifier = xid + neighborhood_size - w.GetWidth() + 1;
    // }

    // if ((int)yid < neighborhood_size) {
    //   y_modifier = neighborhood_size - yid;
    // } else if (yid + neighborhood_size >= w.GetHeight()) {
    //   y_modifier = yid + neighborhood_size - w.GetHeight() + 1;
    // }

    // double denominator = (neighborhood_size*2 + 1 - x_modifier) * (neighborhood_size * 2 + 1 - y_modifier);

    emp::vector<ORG> orgs;

    for (size_t x = std::max((int)xid - neighborhood_size, 0); x <= std::min((int)xid + neighborhood_size, (int)w.GetWidth()-1); x++) {
      for (size_t y = std::max((int)yid - neighborhood_size, 0); y <= std::min((int)yid + neighborhood_size, (int)w.GetHeight()-1); y++) {
        if (w.IsOccupied(x+y*w.GetWidth())) {
          orgs.push_back(w.GetOrg(x,y));
        }
      }
    }

    return ShannonEntropy(orgs);
  }

  // Grid stats
  template <typename ORG>
  emp::vector<emp::vector<double>> GridDensity(emp::World<ORG> & w, int neighborhood_size = 2) {
    emp::vector<emp::vector<double>> densities;
    emp_assert(w.GetAttribute("PopStruct") == "Grid", "Trying to calculate grid statistics on non-grid world. Did you forget to call SetPopStruct_Grid() on this world?");

    densities.resize(w.GetHeight());
    for (size_t y = 0; y < w.GetHeight(); y++) {
      densities[y].resize(w.GetWidth());
      for (size_t x = 0; x < w.GetWidth(); x++) {
        densities[y][x] = GridPointDensity(w, x, y, neighborhood_size);
      }
    }
    return densities;
  }

  template <typename ORG>
  emp::vector<emp::vector<double>> GridShannonEntropy(emp::World<ORG> & w, int neighborhood_size = 2) {
    emp::vector<emp::vector<double>> diversities;
    emp_assert(w.GetAttribute("PopStruct") == "Grid", "Trying to calculate grid statistics on non-grid world. Did you forget to call SetPopStruct_Grid() on this world?");

    diversities.resize(w.GetHeight());
    for (size_t y = 0; y < w.GetHeight(); y++) {
      diversities[y].resize(w.GetWidth());
      for (size_t x = 0; x < w.GetWidth(); x++) {
        diversities[y][x] = GridPointShannonEntropy(w, x, y, neighborhood_size);
      }
    }
    return diversities;
  }
}

#endif // #ifndef EMP_MATH_SPATIAL_STATS_HPP_INCLUDE
