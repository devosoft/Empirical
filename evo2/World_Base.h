//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_BASE_H
#define EMP_EVO_WORLD_BASE_H

#include "../base/Ptr.h"

namespace emp {

  template <typename ORG>
  class World_Base {
  protected:
    using ptr_t = Ptr<ORG>;
    using pop_t = emp::vector<ptr_t>;
    using fit_fun_t = std::function<double(ORG*)>;

    Ptr<Random> random_ptr;
    size_t num_orgs;
    pop_t pop;

  public:
    World_Base() { ; }
    ~World_Base() { ; }
  };

}

#endif



