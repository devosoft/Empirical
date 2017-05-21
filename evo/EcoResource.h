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
  private:
    double inflow;   // Number of resources added per unit time.
    double outflow;  // Proportion of existing resource removed per unit time.

    double level;

  public:
    EcoResource() : inflow(0.0), outflow(0.0), level(0.0) { ; }

    double GetInflow() const { return inflow; }
    double GetOutflow() const { return outflow; }
    double GetLevel() const { return level; }

    EcoResource & SetInflow(double _in) { inflow = _in; return *this; }
    EcoResource & SetOutflow(double _in) { outflow = _in; return *this; }
    EcoResource & SetLevel(double _in) { level = _in; return *this; }

    void Update(double time) { ; }
    void Inject(double amt) { level += amt; }
  };

}
}

#endif
