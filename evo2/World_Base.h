//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_BASE_H
#define EMP_EVO_WORLD_BASE_H

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"
#include "../tools/Random.h"

namespace emp {

  template <typename ORG>
  class World_Base {
  protected:
    using ptr_t = Ptr<ORG>;
    using pop_t = emp::vector<ptr_t>;
    using fit_fun_t = std::function<double(ORG*)>;

    Ptr<Random> random_ptr;  // Random object to use.
    pop_t pop;               // All of the spots in the population.
    size_t num_orgs;         // How many organisms are actually in the population.

  public:
    World_Base() : num_orgs(0) { ; }
    ~World_Base() { Clear(); }

    using value_type = ORG;

    size_t GetSize() const { return pop.size(); }
    size_t GetNumOrgs() const { return num_orgs; }
    Random & GetRandom() { return *random_ptr; }

    void SetRandom(Random & r) { random_ptr = &r; }
  };

}

#endif



