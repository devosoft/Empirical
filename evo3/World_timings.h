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
    size_t last;   // Last update to print, inclusive.

  public:
    World_timings(size_t _f, size_t _s=1, size_t _l=(size_t)-1)
      : first(_f), step(_s), last(_l) { ; }
    World_timings(const World_timings &) = default;
    World_timings(World_timings &&) = default;

    operator=(const World_timings &) = default;
    operator=(World_timings &&) = default;

    size_t GetFirst() const { return first; }
    size_t GetStep() const { return step; }
    size_t GetLast() const { return last; }

    World_timings & SetFirst(size_t _in) { first = _in; return *this; }
    World_timings & SetStep(size_t _in) { step = _in; return *this; }
    World_timings & SetLast(size_t _in) { last = _in; return *this; }

    void TestActive(size_t update) const {
      if (update == first) return true;  // ALWAYS trigger on first, no matter other params.
      if (update < first || update > last || step == 0) return false; // Out of range!
      return (update - first) % step == 0;
    }
  };

}

#endif
