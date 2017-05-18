//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines how individual resources flow into and out of a resource pool.


#ifndef EMP_ECO_RESOURCE_H
#define EMP_ECO_RESOURCE_H

namespace emp {
namespace evo {

  class EcoResource {
    double inflow;   // Number of resources added per unit time.
    double outflow;  // Proportion of existing resource removed per unit time.
  };

}
}

#endif

