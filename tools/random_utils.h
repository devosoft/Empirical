// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_RANDOM_UTILS_H
#define EMP_RANDOM_UTILS_H

#include <vector>

#include "Random.h"

namespace emp {

  template <typename TYPE>
  void shuffle(std::vector<TYPE> & v, Random & random) {
    for (int i = 1; i < (int) v.size(); i++) {
      int new_pos = random.GetInt(i+1);
      if (new_pos == i) continue;
      std::swap(v[i], v[new_pos]);
    }
  }

}

#endif
