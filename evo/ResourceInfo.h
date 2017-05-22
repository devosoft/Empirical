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
    size_t id;
    std::string name;
    double inflow;   // Number of resources added per unit time.
    double outflow;  // Proportion of existing resource removed per unit time.
    std::string desc;

  public:
    ResourceInfo(size_t _id=0, const std::string & _n="", double _in=0.0, double _out=0.0,
                 const std::string & _d="")
      : id(_id), name(_n), inflow(_in), outflow(_out), desc(_d) { ; }
    ResourceInfo(const ResourceInfo &) = default;
    ResourceInfo(ResourceInfo &&) = default;

    ResourceInfo & operator=(const ResourceInfo &) = default;
    ResourceInfo & operator=(ResourceInfo &&) = default;

    size_t GetID() const { return id; }
    const std::string & GetName() const { return name; }
    double GetInflow() const { return inflow; }
    double GetOutflow() const { return outflow; }
    const std::string & GetDesc() const { return desc; }

    ResourceInfo & SetID(size_t _id) { id = _id; return *this; }
    ResourceInfo & SetName(const std::string & _n) { name = _n; return *this; }
    ResourceInfo & SetInflow(double _in) { inflow = _in; return *this; }
    ResourceInfo & SetOutflow(double _in) { outflow = _in; return *this; }
    ResourceInfo & SetDesc(const std::string & _d) { desc = _d; return *this; }

    void Update(double & level) {
      level = level * (1.0 - outflow) + inflow;
    }
  };

}
}

#endif
