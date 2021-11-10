/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file config_setup.hpp
 *  @brief This file provides an example of how to build a configuration class.
 *
 * To create a new config from scratch, the format is:
 *   EMP_BUILD_CONFIG( CLASS_NAME, OPTIONS... )
 *
 * To extend an existing config, simply use:
 *   EMP_EXTEND_CONFIG( NEW_NAME, BASE_CLASS, OPTIONS... )
 *
 * where options can be:
 *
 * GROUP(group name, group description string) -- Start a new group for configuration
 *  options.  Any group structure will be visable to users.
 *
 * VALUE(value name, type, default value, description string, aliases...) -- Create a new
 *  variable in the config object that can be easily accessed, changed, etc.  Zero or more
 *  aliases may be included, which are particularly useful to maintain backward compatibility.
 *
 * CONST(variable name, type, fixed value, description string, aliases...) -- Creates a
 *  configuration constant that cannot be changed (in practice, it is used to allow broader
 * optimizations in the code.)  As with VALUEs, aliases may be included.
 */

#ifndef TESTS_CONFIG_ASSETS_CONFIG_SETUP_HPP_INCLUDE
#define TESTS_CONFIG_ASSETS_CONFIG_SETUP_HPP_INCLUDE

#include "emp/config/config.hpp"

EMP_BUILD_CONFIG( MyConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(DEBUG_MODE, bool, false, "Should we output debug information?"),
  VALUE(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)"),

  GROUP(TEST_GROUP, "These are settings with the sole purpose of testing cConfig.\nFor example, are multi-line descriptions okay?"),
  VALUE(TEST_BOOL, bool, false, "This is a bool value.\nWhat happens\n  ...if we have multiple\n    lines?"),
  VALUE(TEST_STRING, std::string, "default", "This is a string!"),
  CONST(TEST_CONST, int, 91, "This is an unchanging const!"),
  VALUE(TEST_STRING_SPACE, std::string, "abc def   ghi", "This is a string with spaces."),
  VALUE(TEST_STRING_QUOTE, std::string, "\"Quote\"andonemore\"soit'sodd", "This is a string with quote marks."),
  //VALUE(MUTATION_RATE, float, 0.025, "This is my mutation rate.", MUT_RATE),
  VALUE(MUTATION_RATE, float, 0.025, "This is my mutation rate."),
)

#endif // #ifndef TESTS_CONFIG_ASSETS_CONFIG_SETUP_HPP_INCLUDE
