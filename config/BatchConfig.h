//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A tool to control a series of runs and keep them updated.

#ifndef EMP_BATCH_CONFIG_H
#define EMP_BATCH_CONFIG_H

#include "../base/vector.h"

namespace emp {

  /// Class to managae a set of runs:
  /// RUN_T is the type of the object that contains the run info.
  /// CONFIG_T is the config object being used.

  template <typename RUN_T, CONFIG_T>
  class BatchConfig {
  private:
    struct RunInfo {
      size_t id;
      CONFIG_T config;
    };

    emp::vector<RunInfo> runs;

  public:
  };

  // If no config type is explicitly provided, pull it from the run type.
  template <typename RUN_T> using BatchConfig = BatchConfig<RUN_T, RUN_T::config_t>;
}

#endif
