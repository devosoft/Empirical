//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in mechanisms for the organisms to interact with the world.
//  (I.e., how is fitness determined?  How are mutations performed?)

#ifndef EMP_EVO_ORGANISM_MANAGER_H
#define EMP_EVO_ORGANISM_MANAGER_H

namespace emp {
namespace evo {

  template <typename ORG=int>
  class OrgManager_Base {
  protected:
    using ptr_t = ORG *;

  public:
    OrgManager_Base() { ; }
    ~OrgManager_Base() { ; }

    // Allow this and derived classes to be identified as an organism manager.
    static constexpr bool emp_is_organism_manager = true;

    double Fitness(ORG * org) { return (double) *org; }
    bool Mutate(ORG *, emp::Random&) { return false; }
  };

  template <typename ORG=int>
  class OrgManager_Dynamic : public OrgManager_Base<ORG> {
  protected:
  public:
    OrgManager_Dynamic() { ; }
    ~OrgManager_Dynamic() { ; }
  };

  using OrgMBasic  = OrgManager_Base<int>;
  using OrgMDynamic  = OrgManager_Dynamic<int>;
//  using OrgMDirect = OrgManager_Direct<int>;


}
}

#endif
