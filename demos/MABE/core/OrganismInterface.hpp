/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismInterface.h
 *  @brief Class to manage interface between organisms and environment in MABE.
*/

#ifndef MABE_ORGANISM_INTERFACE_H
#define MABE_ORGANISM_INTERFACE_H

#include "BrainWrapper.h"
#include "GenomeWrapper.h"

namespace mabe {

  /// OrganismInterfaceBase provides a consistent interface for unknown organism types.
  class OrganismInterfaceBase {
  protected:
  public:
  };


  /// The OrganismInterface template knows the organism type an environment that need to be linked.
  /// Both ORG_T and ENV_T should be the wrapped types to ensure all components (or defaults) are in place.
  template <typename ORG_T, typename ENV_T>
  class OrganismInterface : public OrganismInterfaceBase {
  protected:
  public:
    OrganismInterface() { ; }
    ~OrganismInterface() { ; }
  };
}

#endif
