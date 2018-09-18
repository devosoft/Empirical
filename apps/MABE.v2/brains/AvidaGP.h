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
    EMP_BUILD_CONFIG( AvidaGPConfig,
      GROUP(DEFAULT_GROUP, "Vector Genome Settings"),
      VALUE(MIN_LENGTH, size_t, 8, "Minimum number of instructions in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of instructions in a genome.")
    )

    AvidaGPConfig config;

  public:
    AvidaGP() { ; }

    std::string GetClassName() const override { return "AvidaGP"; }
    AvidaGPConfig & GetConfig() override { return config; }

    struct compute_t {
    };
  };

}

#endif

