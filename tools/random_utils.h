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
