// This file gets called by Phenotype.h.  It is a wrapper around the custom phenotype_traits file
// that must be built into a project to identify the specific values tracked.  This file ensure
// that all needed macros are handled before calling that file and cleaned up afterward.


// This is the default macro for all phenotypic traits.
#ifndef EMP_ADD_PTRAIT
#define EMP_ADD_PTRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif


// Traits that are accrued, with the last value saved at each divide.
#ifndef EMP_ADD_PTRAIT_BASIC
#define EMP_ADD_PTRAIT_BASIC (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PTRAIT(cur_ ## VAR_NAME, TYPE, DEFAULT, DESC)         \
  EMP_ADD_PTRAIT(last_ ## VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are calculated on divide
#ifndef EMP_ADD_PTRAIT_DIVIDE
#define EMP_ADD_PTRAIT_DIVIDE (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PTRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are calculated on birth
#ifndef EMP_ADD_PTRAIT_BIRTH
#define EMP_ADD_PTRAIT_BIRTH (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PTRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are calculated about an offspring
#ifndef EMP_ADD_PTRAIT_OFFSPRING
#define EMP_ADD_PTRAIT_OFFSPRING (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PTRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif


// If we have not definted another place to find traits, used "phenotype_traits.h"
#ifndef EMP_PHENOTYPE_FILE
#define EMP_PHENOTYPE_FILE "phenotype_traits.h"
#endif

#include EMP_PHENOTYPE_FILE


// Remove all macros for next cycle.
#undef EMP_ADD_PTRAIT
#undef EMP_ADD_PTRAIT_BASIC
#undef EMP_ADD_PTRAIT_DIVIDE
#undef EMP_ADD_PTRAIT_BIRTH
#undef EMP_ADD_PTRAIT_OFFSPRING

