// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

// This file gets called by config.h.  It is a wrapper around the custom config_opts file
// that will be built into each project to ensure that all needed macros are handled before
// calling that file and cleaned up afterward.

// Setup default values for all unset macros.
#ifndef EMP_CONFIG_GROUP
#define EMP_CONFIG_GROUP(NAME, DESC)
#endif

#ifndef EMP_CONFIG_VAR
#define EMP_CONFIG_VAR(NAME, TYPE, DEFAULT, DESC)
#endif

#ifndef EMP_CONFIG_ALIAS
#define EMP_CONFIG_ALIAS(ALIAS)
#endif

#ifndef EMP_CONFIG_CONST
#define EMP_CONFIG_CONST(NAME, TYPE, VALUE, DESC)
#endif

// If EMP_CONFIG_FILE is set at compile time, the filename given will be used to define
// configuration options.  Otherwise "config_opts.h" is assumed to be the config file.
#ifndef EMP_CONFIG_FILE
#define EMP_CONFIG_FILE "config_opts.h"
#endif


// Call the actual configuration file.
#include EMP_CONFIG_FILE

// Clean up all of the macros we were just using.
#undef EMP_CONFIG_GROUP
#undef EMP_CONFIG_VAR
#undef EMP_CONFIG_ALIAS
#undef EMP_CONFIG_CONST
