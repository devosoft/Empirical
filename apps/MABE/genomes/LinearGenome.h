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
  public:
    LinearGenome() { ; }

    std::string GetClassName() const override { return "LinearGenome"; }

    struct data_t {
      emp::vector<unsigned char> data;
    };
  };

}

#endif

