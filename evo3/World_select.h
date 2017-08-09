//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Handle selection methods for worlds.

#ifndef EMP_EVO_WORLD_SELECT_H
#define EMP_EVO_WORLD_SELECT_H

#include <map>

#include "../base/assert.h"

namespace emp {

  template<typename ORG> class World;

  // Elite Selection picks a set of the most fit individuals from the population to move to
  // the next generation.  Find top e_count individuals and make copy_count copies of each.
  template<typename ORG>
  void EliteSelect(World<ORG> & world, size_t e_count, size_t copy_count) {
    emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
    emp_assert(copy_count > 0);

    // Load the population into a multimap, sorted by fitness.
    std::multimap<double, size_t> fit_map;
    for (size_t id = 0; id < world.GetSize(); id++) {
      if (world.IsOccupied(id)) {
        const double cur_fit = world.CalcFitnessID(id);
        fit_map.insert( std::make_pair(cur_fit, id) );
      }
    }

    // Grab the top fitnesses and move them into the next generation.
    auto m = fit_map.rbegin();
    for (size_t i = 0; i < e_count; i++) {
      const size_t repro_id = m->second;
      world.DoBirth( world[repro_id], repro_id, copy_count);
      ++m;
    }
  }


}

#endif
