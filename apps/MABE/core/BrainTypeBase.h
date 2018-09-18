/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  BrainTypeBase.h
 *  @brief Base class for all brain types.
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

#ifndef MABE_BRAIN_TYPE_BASE_H
#define MABE_BRAIN_TYPE_BASE_H

#include <string>

#include "config/config.h"

namespace mabe {

  class BrainTypeBase {
  private:
    std::string name;
  public:
    BrainTypeBase() { ; }
    virtual ~BrainTypeBase() { ; }

    virtual std::string GetClassName() const = 0;
    virtual emp::Config & GetConfig() = 0;

    const std::string & GetName() { return name; }
    void SetName(const std::string & in_name) { name = in_name; }
  };

}

#endif

