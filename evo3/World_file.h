//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines extra details needed for world data files.


#ifndef EMP_EVO_WORLD_FILE_H
#define EMP_EVO_WORLD_FILE_H

#include <functional>

#include "../data/DataFile.h"

namespace emp {

  class World_file : public DataFile {
  public:
    using time_fun_t = std::function<bool(size_t)>;
    time_fun_t timing_fun;

  public:
    World_file(const std::string & filename)
      : DataFile(filename), timing_fun([](size_t){return true;}) { ; }

    void Update(size_t update) {
      if (timing_fun(update)) DataFile::Update();
    }

    void SetTiming(time_fun_t fun) { timing_fun = fun; }
    void SetTimingOnce(size_t print_time) {
      timing_fun = [print_time](size_t update) { return update == print_time; };
    }
    void SetTimingRepeat(size_t step) {
      emp_assert(step > 0);
      timing_fun = [step](size_t update) { return update % step == 0; };
    }
    void SetTimingRange(size_t first, size_t step, size_t last) {
      emp_assert(step > 0);
      emp_assert(first < last);
      timing_fun = [first,step,last](size_t update) {
	      if (update < first || update > last) return false;
	      return ((update - first) % step) == 0;
      };
    }
  };

}

#endif
