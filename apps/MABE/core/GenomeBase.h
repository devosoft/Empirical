/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  GenomeBase.h
 *  @brief Base class for all genome types.
 *
 *  Genomes are generic ways of encoding Brains or other aspects of organisms.
 */

#ifndef MABE_GENOME_BASE_H
#define MABE_GENOME_BASE_H

namespace mabe {

  class GenomeBase {
  private:
  public:
    GenomeBase() { ; }
    virtual ~GenomeBase() { ; }

    virtual std::string GetClassName() const = 0;
  };

}

#endif

