// This file gets called by Phenotype.h.  It is a wrapper around the custom phenotype_traits file
// that must be built into a project to identify the specific values tracked.  This file ensure
// that all needed macros are handled before calling that file and cleaned up afterward.


// This is the default macro for all phenotypic traits.
#ifndef EMP_ADD_PHENO_TRAIT
#define EMP_ADD_PHENO_TRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif


// Traits that are accrued, with the last value saved at each divide
#ifndef EMP_ADD_PHENO_TRAIT_LOGGED
#define EMP_ADD_PHENO_TRAIT_BASIC (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PHENO_TRAIT(cur_ ## VAR_NAME, TYPE, DEFAULT, DESC)         \
  EMP_ADD_PHENO_TRAIT(last_ ## VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are updated throughout an organisms' lifetime
#ifndef EMP_ADD_PHENO_TRAIT_BASIC
#define EMP_ADD_PHENO_TRAIT_DIVIDE (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PHENO_TRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are calculated on divide
#ifndef EMP_ADD_PHENO_TRAIT_DIVIDE
#define EMP_ADD_PHENO_TRAIT_BIRTH (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PHENO_TRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif

// Traits that are calculated on birth and never changed
#ifndef EMP_ADD_PHENO_TRAIT_LOCKED
#define EMP_ADD_PHENO_TRAIT_OFFSPRING (VAR_NAME, TYPE, DEFAULT, DESC)    \
  EMP_ADD_PHENO_TRAIT(VAR_NAME, TYPE, DEFAULT, DESC)
#endif


// If we have not definted another place to find traits, used "phenotype_traits.h"
#ifndef EMP_PHENOTYPE_FILE
#define EMP_PHENOTYPE_FILE "phenotype_traits.h"
#endif

#include EMP_PHENOTYPE_FILE


// Remove all macros for next cycle.
#undef EMP_ADD_PHENO_TRAIT
#undef EMP_ADD_PHENO_TRAIT_TRACKED
#undef EMP_ADD_PHENO_TRAIT_ASSESSED
#undef EMP_ADD_PHENO_TRAIT_INHERETED
#undef EMP_ADD_PHENO_TRAIT_OFFSPRING

