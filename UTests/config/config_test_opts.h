// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

/////////////////////////////////////////////////////////////////////////////////////////
// This file defines all of the configuration options available.  It is read in several
// times from config.h, each with different macros associated with the command below.
//
// The available commands are:
//
// EMP_CONFIG_GROUP(group name, group description string) -- This command is only needed
//  if you want to break configuration options into groups.  Any group structure will be
//  preserved when user-accessible configuration options are presented.
//
// EMP_CONFIG_VAR(variable name, type, default value, description string) -- This command
//  allows you to create a new command in the cConfig object that can be easily accessed,
//  changed, etc.
//
// EMP_CONFIG_ALIAS(alias name) -- Include an alias for the previous variable.  This is
//  particularly useful to maintain backward compatibility if you need to change a
//  variable name in a new release of software
//
// EMP_CONFIG_CONST(variable name, type, fixed value, description string) -- This command
//  creates a configuration constant that cannot be changed.  In practice, it is used to
//  allow broader optimizations in the code.

EMP_CONFIG_GROUP(DEFAULT_GROUP, "General Settings")
EMP_CONFIG_VAR(DEBUG_MODE, bool, false, "Should we output debug information?")
EMP_CONFIG_VAR(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)")

EMP_CONFIG_GROUP(TEST_GROUP, "These are settings with the sole purpose of testing cConfig.\nFor example, are multi-line descriptions okay?")
EMP_CONFIG_VAR(TEST_BOOL, bool, false, "This is a bool value.\nAnd what happens\n  ...if we have multiple\n    lines?")
EMP_CONFIG_VAR(TEST_STRING, std::string, "default", "This is a string!")
EMP_CONFIG_CONST(TEST_CONST, int, 91, "This is an unchanging const!")
EMP_CONFIG_VAR(TEST_STRING_SPACE, std::string, "abc def   ghi", "This is a string with spaces.")
EMP_CONFIG_VAR(MUTATION_RATE, float, 0.025, "This is my mutation rate.")
