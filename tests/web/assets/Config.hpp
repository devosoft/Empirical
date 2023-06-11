/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Config.hpp
 *
 */

#ifndef TESTS_WEB_ASSETS_CONFIG_HPP_INCLUDE
#define TESTS_WEB_ASSETS_CONFIG_HPP_INCLUDE

#include "emp/config/config.hpp"

// Config class for testing purposes.
// If edits are made here, changes may need
// to be made in the tests in ConfigPanel.cc.

EMP_BUILD_CONFIG( Config,
  GROUP(MAIN, "Global settings"),
  VALUE(BOOL_EX, bool, true, "example description"),
  VALUE(SEED, int, -1, "Random number generator seed"),
  VALUE(RADIATION_PRESCRIPTION_FILE, std::string, "none", "File containing radiation prescription"),

  GROUP(CELL, "Cell settings"),
  VALUE(NEUTRAL_MUTATION_RATE, double, .05, "Probability of a neutral mutation (only relevant for phylogenetic signature)"),
  VALUE(ASYMMETRIC_DIVISION_PROB, double, 0, "Probability of a change in stemness"),

  GROUP(TREATMENT, "Treatment settings"),
  VALUE(RADIATION_DOSES, int, 1, "Number of radiation doses to apply (for use in web interface - use a radiation prescription file for command-line)"),
  VALUE(RADIATION_DOSE_SIZE, double, 2, "Dose size (Gy) (for use in web interface - use a radiation prescription file for command-line)"),
  VALUE(UNKNOWN, std::string, "A unsupported type", "Type overridden later to simulate new config type"),
);

#endif // #ifndef TESTS_WEB_ASSETS_CONFIG_HPP_INCLUDE
