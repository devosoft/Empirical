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

#include "core/GenomeTypeBase.h"

namespace mabe {

  template <typename T>
  class VectorGenome : public GenomeTypeBase {
  private:
    EMP_BUILD_CONFIG( VGConfig,
      GROUP(DEFAULT_GROUP, "Vector Genome Settings"),
      VALUE(ALPHABET_SIZE, size_t, 32, "Number of possible values in genome."),
      VALUE(DEFAULT_LENGTH, size_t, 64, "Default number of sites in a genome"),
      VALUE(MIN_LENGTH, size_t, 1, "Minimum number of sites in a genome."),
      VALUE(MAX_LENGTH, size_t, 1024, "Maximum number of sites in a genome.")
    )

    VGConfig config;

  public:
    VectorGenome() { ; }

    std::string GetClassName() const override { return "VectorGenome"; }
    VGConfig & GetConfig() override { return config; }

    using genome_t = emp::vector<T>;

    void Randomize(emp::Random & random, genome_t & data) {
      const size_t genome_size = config.DEFAULT_LENGTH();
      data.resize(genome_size);
      for (size_t i = 0; i < genome_size; i++) data[i] = random.GetUInt( config.ALPHABET_SIZE() );
    }

    void Print(std::ostream & os, genome_t & data) const { os << "No Print Function Available"; }
  };

}

#endif

