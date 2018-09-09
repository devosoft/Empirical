/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  NKLandscape.h
 *  @brief An NK landscape to simplify NK-based studies.
 */

#ifndef MABE_NK_LANDSCAPE_H
#define MABE_NK_LANDSCAPE_H

#include "tools/BitVector.h"

#include "core/EnvironmentBase.h"

namespace mabe {

  class NKLandscape : public EnvironmentBase {
  private:
    EMP_BUILD_CONFIG( NKConfig,
      GROUP(DEFAULT_GROUP, "NK Landscape Settings"),
      VALUE(N, size_t, 100, "Number of genes in landscape"),
      VALUE(K, size_t, 7, "Number of additional bits per gene (eg, K=7 means 8 bits per gene)")
    )  

    NKConfig config;
  public:
    NKLandscape(const std::string & name) : EnvironmentBase(name) { ; }

    std::string GetClassName() const override { return "NKLandscape"; }

    /// Required accessor for configuration objects.
    NKConfig & GetConfig() override { return config; }

    void Setup(EvolverBase & world) override {
      // @CAO: We should change this to a BitVector reference when decay is setup for typing.
      std::function<double(emp::BitVector)> fit_fun = [](emp::BitVector bv){ return 0.0; };
      world.AddOrgData<double>("fitness", 0.0, fit_fun,
                               "double(const BitVector &)", "NK Fitness Function.");
    }

  };

}

#endif

