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

#include "core/GenomeTypeBase.h"

namespace mabe {

  class LinearGenome : public GenomeTypeBase {
  private:
    EMP_BUILD_CONFIG( LGConfig,
      GROUP(DEFAULT_GROUP, "Linear Genome Settings"),
      VALUE(ALPHABET_SIZE, size_t, 256, "Number of possible values in genome."),
      VALUE(DEFAULT_LENGTH, size_t, 64, "Default number of sites in a genome"),
      VALUE(MIN_LENGTH, size_t, 8, "Minimum number of sites in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of sites in a genome.")
    )

    LGConfig config;
  public:
    LinearGenome() { ; }

    std::string GetClassName() const override { return "LinearGenome"; }
    LGConfig & GetConfig() override { return config; }

    using genome_t = emp::vector<unsigned char>;

    void Randomize(emp::Random & random, genome_t & data) {
      data.resize( config.DEFAULT_LENGTH() );
      for (auto & x : data) x = (unsigned char) random.GetUInt( config.ALPHABET_SIZE() );
    }

    void Print(std::ostream & os, genome_t & data) const { os << "No Print Function Available"; }
  };

}

#endif

