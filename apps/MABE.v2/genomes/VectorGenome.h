/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  VectorGenome.h
 *  @brief The most generic form of a genome.
 */

#ifndef MABE_VECTOR_GENOME_H
#define MABE_VECTOR_GENOME_H

#include "base/vector.h"

#include "core/GenomeBase.h"

namespace mabe {

  template <typename T>
  class VectorGenome : public GenomeBase {
  private:
    EMP_BUILD_CONFIG( VGConfig,
      GROUP(DEFAULT_GROUP, "Vector Genome Settings"),
      VALUE(ALPHABET_SIZE, size_t, 2, "Number of possible values in genome."),
      VALUE(MIN_LENGTH, size_t, 1, "Minimum number of sites in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of sites in a genome.")
    )

    VGConfig config;

  public:
    VectorGenome() { ; }

    std::string GetClassName() const override { return "VectorGenome"; }
    VGConfig & GetConfig() override { return config; }

    using data_t = emp::vector<T>;
  };

}

#endif

