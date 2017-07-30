//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines timings for events in emp::World objects.


#ifndef EMP_EVO_WORLD_TIMINGS_H
#define EMP_EVO_WORLD_TIMINGS_H

namespace emp {

  class World_timings {
  private:
    size_t first;  // First update to print.
    size_t step;   // How frequently to print (0 means only print once.)
    size_t last;   // Last update to print.

  public:
    World_timings(size_t _f, size_t _s, size_t _l) : first(_f), step(_s), last(_l) { ; }
  };

}

#endif
