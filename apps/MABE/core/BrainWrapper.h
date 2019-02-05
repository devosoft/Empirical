/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  BrainWrapper.h
 *  @brief A template wrapper for a brain object that will simplify access to optional traits.
 *
 *  Brains are basic forms of agent contollers.
 * 
 *  Each derived brain class much have a brain_t type indicating the information it needs to
 *  store in organisms.  It must also have the following functions:
 * 
 *    std::string GetClassName()
 *    emp::Config & GetConfig()
 *    void Print(std::ostream & os, brain_t & brain) const
 *    void OnBeforeRepro(brain_t & parent_brain)
 *    void OnOffspringReady(brain_t & offspring_brain, brain_t & parent_brain)
 *    void OnInjectReady(brain_t & brain)
 *    void OnBeforePlacement(brain_t & brain)
 *    void OnPlacement(brain_t & brain)
 *    void OnOrgDeath(brain_t & brain)

 */

#ifndef MABE_BRAIN_WRAPPER_H
#define MABE_BRAIN_WRAPPER_H

#include <string>

#include "config/config.h"
#include "meta/ConceptWrapper.h"

namespace mabe {

  EMP_BUILD_CONCEPT( BrainWrapper, BrainBase,
    OPTIONAL_FUN(GetClassName, "Unnamed Brain", std::string),
    OPTIONAL_FUN(GetConfig, emp::GetEmptyConfig(), emp::Config &),
    OPTIONAL_FUN(Randomize, false, bool, emp::Random &),
    OPTIONAL_FUN(Print, false, bool),
    OPTIONAL_FUN(OnBeforeRepro, , void),                  // Brain about to be reproduced.
    OPTIONAL_FUN(OnOffspringReady, , void, BrainBase &),  // Brain offspring; arg is parent brain.
    OPTIONAL_FUN(OnInjectReady, , void),                  // Brain about to be injected.
    OPTIONAL_FUN(OnBeforePlacement, , void),              // Brain about to be placed
    OPTIONAL_FUN(OnPlacement, , void),                    // Brain just placed.
    OPTIONAL_FUN(OnOrgDeath, , void)                      // Brain about to die.
  );

}

#endif

