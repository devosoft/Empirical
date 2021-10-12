/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file test-macro.hpp
 *  @brief This is an auto-generated file that defines a set of configuration options.
 *  To create a new config from scratch, the format is:
 *    EMP_BUILD_CONFIG( CLASS_NAME, OPTIONS... )
 *
 *  To extend an existing config, simply use:
 *    EMP_EXTEND_CONFIG( NEW_NAME, BASE_CLASS, OPTIONS... )
 *
 *  The available OPTIONS are:
 *
 *  GROUP(group name, group description string)
 *   Start a new group of configuration options.  Group structure is preserved
 *   when user-accessible configuration options are generated.
 *
 *  VALUE(variable name, type, default value, description string)
 *   Create a new setting in the emp::Config object that can be easily accessed.
 *
 *  CONST(variable name, type, fixed value, description string)
 *   Create a new configuration constant that cannot be changed.  In practice,
 *   allows broader optimizations in the code.
 *
 *  ALIAS(alias name)
 *   Include an alias for the previous setting.  This command is useful to
 *   maintain backward compatibility if names change in newer software versions.
 */

#ifndef EXAMPLES_CONFIG_TEST_MACRO_HPP_INCLUDE
#define EXAMPLES_CONFIG_TEST_MACRO_HPP_INCLUDE

#include "emp/config/config.hpp"

EMP_BUILD_CONFIG(MyConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
    VALUE(DEBUG_MODE, bool, 0, "Should we output debug information?"),
    VALUE(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)"),

  GROUP(TEST_GROUP, "These are settings with the sole purpose of testing cConfig.
For example, are multi-line descriptions okay?"),
    VALUE(TEST_BOOL, bool, 0, "This is a bool value.\nWhat happens\n  ...if we have multiple\n    lines?"),
    VALUE(TEST_STRING, std::string, "default", "This is a string!"),
    CONST(TEST_CONST, int, 91, "This is an unchanging const!"),
    VALUE(TEST_STRING_SPACE, std::string, "abc def   ghi", "This is a string with spaces."),
    VALUE(MUTATION_RATE, float, 0.025000, "This is my mutation rate."),

)

#endif // #ifndef EXAMPLES_CONFIG_TEST_MACRO_HPP_INCLUDE
