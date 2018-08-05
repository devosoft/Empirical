/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  TrackLineage.h
 *  @brief Tools for tracking the lineages leading to the active population.
 */

#ifndef MABE_TRACK_LINEAGE_H
#define MABE_TRACK_LINEAGE_H

#include "core/SchemaBase.h"

namespace mabe {

  class TrackLineage : public SchemaBase {
  private:
  public:
    TrackLineage(const std::string & name) : SchemaBase(name) { ; }

    std::string GetClassName() const override { return "TrackLineage"; }
  };

}

#endif

