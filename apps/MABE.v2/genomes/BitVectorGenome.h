/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  BitVectorGenome.h
 *  @brief Genomes consisting of a series of bits.
 */

#ifndef MABE_BIT_VECTOR_GENOME_H
#define MABE_BIT_VECTOR_GENOME_H

#include "tools/BitVector.h"

#include "core/GenomeBase.h"

namespace mabe {

  class BitVectorGenome : public GenomeBase {
  private:
    EMP_BUILD_CONFIG( BVConfig,
      GROUP(DEFAULT_GROUP, "BitVector Genome Settings"),
      VALUE(MIN_LENGTH, size_t, 8, "Minimum number of sites in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of sites in a genome.")
    )

    BVConfig config;
  public:
    BitVectorGenome() { ; }

    std::string GetClassName() const override { return "BitVectorGenome"; }
    LGConfig & GetConfig() override { return config; }

    using data_t = emp::BitVector;
  };

}

#endif

