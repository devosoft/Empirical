//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines the base class for a World template for use in evolutionary algorithms.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <functional>

#include "../base/vector.h"

namespace emp {

  template <typename ORG, typename GENOTYPE=ORG>
  class World {
  private:
    using ptr_t = Ptr<ORG>;
    using pop_t = emp::vector<ptr_t>;
    using fit_fun_t = std::function<double(ORG&)>;

    Ptr<Random> random_ptr;   // Random object to use.
    bool random_owner;        // Did we create our own random number generator?
    pop_t pop;                // All of the spots in the population.
    size_t num_orgs;          // How many organisms are actually in the population.

    fit_fun_t fun_calc_fitness;                           // Fitness function
    std::function<size_t(Ptr<ORG>)> fun_add_org;          // Technique to inject a new organism.
    std::function<size_t(Ptr<ORG>, size_t)> fun_add_org;  // Technique to add a new offspring.
    

    // AddOrgAt & AddOrgAppend are the only ways to add organisms (others must go through these)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);
    size_t AddOrgAppend(Ptr<ORG> new_org);

    // Build a Setup function in world that calls ::Setup() on whatever is passed in IF it exists.
    EMP_CREATE_OPTIONAL_METHOD(SetupOrg, Setup);


  public:
    World() { ; }
  };

}

#endif
