/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  LinearGenome.h
 *  @brief The most generic form of a genome.
 */

#ifndef MABE_LINEAR_GENOME_H
#define MABE_LINEAR_GENOME_H

#include "core/GenomeBase.h"

namespace mabe {

  class LinearGenome : public GenomeBase {
  private:
    EMP_BUILD_CONFIG( LGConfig,
      GROUP(DEFAULT_GROUP, "Linear Genome Settings"),
      VALUE(ALPHABET_SIZE, size_t, 256, "Number of possible values in genome."),
      VALUE(MIN_LENGTH, size_t, 8, "Minimum number of sites in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of sites in a genome.")
    )

    LGConfig config;
  public:
    LinearGenome() { ; }

    std::string GetClassName() const override { return "LinearGenome"; }
    LGConfig & GetConfig() override { return config; }

    using data_t = emp::vector<unsigned char>;
  };

}

#endif

