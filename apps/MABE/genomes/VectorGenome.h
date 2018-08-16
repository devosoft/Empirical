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
  public:
    VectorGenome() { ; }

    std::string GetClassName() const override { return "VectorGenome"; }

    struct data_t {
      emp::vector<T> data;
    };
  };

}

#endif

