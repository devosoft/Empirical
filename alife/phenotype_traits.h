//////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains a default set of phenotyic tratis.  In practice, these should be
//  specific to each project.
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
//  Any default can be a fixed value or an equation.  If an equation, it can use the variables
//     Phenotype * parent_pheno  (== NULL if no parent exists)
//     Config & config
//     Environment & environment
//     World & world
//  

EMP_ADD_PHENO_TRAIT_BASIC(NumOffspring, int, 0,
                          "Count of offspring produced by this organism");
EMP_ADD_PHENO_TRAIT_BASIC(HasParasite, bool, false,
                          "Has this organism been infected by a parasite?");

EMP_ADD_PHENO_TRAIT_LOGGED(MetabolicRate, double, 1.0,
                           "Speed at which organism can process instructions");
EMP_ADD_PHENO_TRAIT_LOGGED(TaskCounts, std::vector<int>, std::vector<int>(environment.NumTasks(),0),
                           "How many times has this organism performed each task?");

EMP_ADD_PHENO_TRAIT_DIVIDE(GestationTime, int, parent_pheno ?
                           (parent_pheno->LastDivideCycles() - parent_pheno.CurDivideCycles()) : 0,
                           "Number of CPU cycles to produce an offspring.");
EMP_ADD_PHENO_TRAIT_DIVIDE(Energy, double, 0.0, parent_pheno->GetEnergy()/2.0,
                           "Amount of energy organism has available to spend.");
EMP_ADD_PHENO_TRAIT_DIVIDE(LastDivideUpdate, int, 0);
EMP_ADD_PHENO_TRAIT_DIVIDE(LastDivideCycles, int, 0);

EMP_ADD_PHENO_TRAIT_LOCKED(Generation, int, 0);
EMP_ADD_PHENO_TRAIT_LOCKED(PhylogeneticDepth, int, 0);
EMP_ADD_PHENO_TRAIT_LOCKED(ParentTrue, bool, false);
EMP_ADD_PHENO_TRAIT_LOCKED(DriftVar, double, 0.0);
EMP_ADD_PHENO_TRAIT_LOCKED(UpdateBorn, int, 0.0);
