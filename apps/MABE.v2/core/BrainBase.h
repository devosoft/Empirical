/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  BrainBase.h
 *  @brief Base class for all brain types.
 *
 *  Brains are basic forms of agent contollers.
 */

#ifndef MABE_BRAIN_BASE_H
#define MABE_BRAIN_BASE_H

#include <string>

#include "config/config.h"

namespace mabe {

  class BrainBase {
  private:
    std::string name;
  public:
    BrainBase() { ; }
    virtual ~BrainBase() { ; }

    virtual std::string GetClassName() const = 0;
    virtual emp::Config & GetConfig() = 0;

    const std::string & GetName() { return name; }
    void SetName(const std::string & in_name) { name = in_name; }
  };

}

#endif

