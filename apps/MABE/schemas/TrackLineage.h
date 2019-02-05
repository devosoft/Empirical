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
    EMP_BUILD_CONFIG( TLConfig,
      GROUP(DEFAULT_GROUP, "Track Linage Settings"),
      VALUE(OUTPUT_FREQUENCY, size_t, 1000, "Number of generation between outputs."),
    )  

    TLConfig config;
  public:
    TrackLineage(const std::string & name) : SchemaBase(name) { ; }

    std::string GetClassName() const override { return "TrackLineage"; }

    /// Required accessor for configuration objects.
    emp::Config & GetConfig() override { return config; }
  };

}

#endif

