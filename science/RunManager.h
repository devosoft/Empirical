//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A tool to control a series of runs and keep them updated.

#ifndef EMP_RUN_MANAGER_H
#define EMP_RUN_MANAGER_H

namespace emp {

  template <typename RUN_T, CONFIG_T>
  class RunManager {
  private:
    struct RunInfo {
      size_t id;
      CONFIG_T config;
    };

  public:
  };

  // If no config type is explicitly provided, pull it from the run type.
  template <typename RUN_T> using RunManager = RunManager<RUN_T, RUN_T::config_t>;
}

#endif
