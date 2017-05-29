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

    // AddOrgAt & AddOrgAppend are the only ways to add organisms (others must go through these)
    size_t AddOrgAt(Ptr<ORG> new_org, size_t pos);
    size_t AddOrgAppend(Ptr<ORG> new_org);

    // The following functions call focus on context (and will be overridden as needed).
    // AddOrg inserts an organism from OUTSIDE of the population.
    // AddOrgBirth inserts an organism that was born INSIDE the population.
    size_t AddOrg(Ptr<ORG> new_org) { return AddOrgAppend(new_org); }

    size_t AddOrgBirth(Ptr<ORG> new_org, size_t parent_pos) {
      emp_assert(random_ptr); // Random must be set before being used.
      const size_t pos = random_ptr->GetUInt(pop.size());
      return AddOrgAt(new_org, pos);
    }

  public:
    World_Base() : num_orgs(0) { ; }
    ~World_Base() { Clear(); }

    using value_type = ORG;

    size_t GetSize() const { return pop.size(); }
    size_t GetNumOrgs() const { return num_orgs; }
    Random & GetRandom() { return *random_ptr; }

    void SetRandom(Random & r) { random_ptr = &r; }

    void Clear();
    void ClearOrgAt(size_t pos);
    
    void Resize(size_t new_size) {
      for (size_t i = new_size; i < pop.size(); i++) ClearOrgAt(i); // Remove orgs past new size.
      pop.resize(new_size, nullptr);                                // Default new orgs to null.
    }
  };

  
  size_t World_Base::AddOrgAt(Ptr<ORG> new_org, size_t pos) {
    emp_assert(pos < pop.size());   // Make sure we are placing into a legal position.
    if (pop[pos]) { pop[pos].Delete(); --num_orgs; }
    pop[pos] = new_org;
    fitM.ClearAt(pos);
    ++num_orgs;
    return pos;
  }
  
  size_t World_Base::AddOrgAppend(Ptr<ORG> new_org) {
    const size_t pos = pop.size();
    pop.push_back(new_org);
    fitM.ClearAt(pos);
    ++num_orgs;
    return pos;
  }

  // Delete all organisms.
  void World_Base::Clear() {
    for (ptr_t org : pop) if (org) org.Delete();  // Delete current organisms.
    pop.resize(0);                              // Remove deleted organisms.
    num_orgs = 0;
  }

  // Delete organism at a specified position.
  void World_Base::ClearOrgAt(size_t pos) {
    if (!pop[pos]) return;  // No organism; no need to do anything.
    pop[pos].Delete();
    pop[pos]=nullptr;
    num_orgs--;
  }

}

#endif



