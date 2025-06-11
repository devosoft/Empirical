/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/config/config_utils.hpp
 * @brief Helper functions for working with emp::Config objects.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_CONFIG_CONFIG_UTILS_HPP_GUARD
#define INCLUDE_EMP_CONFIG_CONFIG_UTILS_HPP_GUARD

#include "config.hpp"

namespace emp {

  [[maybe_unused]] static emp::Config & GetEmptyConfig() {
    static emp::Config empty_config;
    return empty_config;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_CONFIG_CONFIG_UTILS_HPP_GUARD
