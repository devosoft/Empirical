#ifndef EMP_PHENOTYPE_H
#define EMP_PHENOTYPE_H

/////////////////////////////////////////////////////////////////////////////////////////////
//
//  Phenotype collects various types of data about the state of an organism
//
//  These traits are a set of values that are calculated based on an organism's activities
//  or those of its ancestors.
//
//  The types of traits available are:
//    BASIC   <- Traits start at default and adjust over the lifetime of an organism
//    LOGGED  <- Like BASIC, but accumulated value at previous divide is stored
//    DIVIDE  <- Traits are calculated when offspring are produced
//    LOCKED  <- Traits are calculated and set at birth and never changed
//
//  Phenotypic traits are defined as:
//    EMP_ADD_PHENO_TRAIT_***(name, type, default, description)
//

namespace emp {

  class Phenotype {
  };

};


#endif
