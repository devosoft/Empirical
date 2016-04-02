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

    emp::vector<MEMBER *> pop;
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
