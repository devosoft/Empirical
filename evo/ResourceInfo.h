//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file defines how individual resources flow into and out of a resource pool.


#ifndef EMP_RESOURCE_INFO_H
#define EMP_RESOURCE_INFO_H

#include <string>

namespace emp {
namespace evo {

  class ResourceInfo {
  private:
    std::string name;
    std::string desc;
    double inflow;   // Number of resources added per unit time.
    double outflow;  // Proportion of existing resource removed per unit time.

  public:
    ResourceInfo(const std::string & _n, double _in=0.0, double _out=0.0, const std::string & _d="")
      : name(_n), inflow(_in), outflow(_out), desc(_d) { ; }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    double GetInflow() const { return inflow; }
    double GetOutflow() const { return outflow; }

    ResourceInfo & SetName(const std::string & _n) { name = _n; return *this; }
    ResourceInfo & SetDesc(const std::string & _d) { desc = _d; return *this; }
    ResourceInfo & SetInflow(double _in) { inflow = _in; return *this; }
    ResourceInfo & SetOutflow(double _in) { outflow = _in; return *this; }

    void Update() {
      level = level * (1.0 - outflow) + inflow;
    }
    void Inject(double amt) { level += amt; }
  };

}
}

#endif
