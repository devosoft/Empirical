/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  config_utils.hpp
 *  @brief Helper functions for working with emp::Config objects.
 *
 */

#ifndef EMP_CONFIG_UTILS_H
#define EMP_CONFIG_UTILS_H

#include "config.hpp"

namespace emp {

  static emp::Config & GetEmptyConfig() {
    static emp::Config empty_config;
    return empty_config;
  }

}

#endif
