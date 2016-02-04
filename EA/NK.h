//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build NK-based algorithms.

#ifndef NK_H
#define NK_H

#include <array>

#include "../tools/const_utils.h"

namespace emp {
namespace EA {

  template <int N, int K>
  class NKLandscape {
    static constexpr int count = N * emp::constant::IntPow(2,K);
    std::array<double, count> landscape;
  };
  
}
}
  
#endif

