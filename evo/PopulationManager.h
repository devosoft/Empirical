//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines built-in population manages for use with emp::evo::World

#ifndef EMP_EVO_POPULATION_MANAGER_H
#define EMP_EVO_POPULATION_MANAGER_H

namespace emp {
namespace evo {

  template <typename MEMBER>
  struct PopulationManager_Base {
    // Allow this and derived classes to be identified as a population manager.
    static constexpr bool emp_is_population_manager = true;

    using ptr_t = MEMBER *;
    emp::vector<MEMBER *> pop;

    ptr_t & operator[](int i) { return pop[i]; }
    const ptr_t operator[](int i) const { return pop[i]; }

    uint32_t size() const { return pop.size(); }
    void resize(int new_size) { pop.resize(new_size); }
  };

  // A standard population manager for using sychronous generations in a traditional
  // evolutionary algorithm setup.

  template <typename MEMBER>
  struct PopulationManager_EA : public PopulationManager_Base<MEMBER> {
    emp::vector<MEMBER *> next_pop;
  };

}
}


#endif
