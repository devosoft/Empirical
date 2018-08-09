/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  AvidaGP.h
 *  @brief Brain usng AvidaGP virtual CPU
 */

#ifndef MABE_AVIDA_GP_H
#define MABE_AVIDA_GP_H

#include "core/BrainBase.h"

namespace mabe {

  class AvidaGP : public BrainBase {
  private:
  public:
    AvidaGP() { ; }

    std::string GetClassName() const override { return "AvidaGP"; }

    struct compute_t {
    };
  };

}

#endif

