/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  GenomeWrapper.h
 *  @brief A template wrapper for a genome object that will simplify access to optional traits.
 *
 *  Genomes are generic ways of encoding info for Brains or other aspects of organisms.
 *
 *  Each genome class must contain any heritable material for an organism.  It must also have any
 *  of the following functions:
 * 
 *    std::string GetClassName()
 *    emp::Config & GetConfig()
 *    void Randomize(emp::Random & random, genome_t & genome)
 *    void Print(std::ostream & os, genome_t & genome) const
 *    void OnBeforeRepro(genome_t & parent_genome)
 *    void OnOffspringReady(genome_t & offspring_genome, genome_t & parent_genome)
 *    void OnInjectReady(genome_t & genome)
 *    void OnBeforePlacement(genome_t & genome)
 *    void OnPlacement(genome_t & genome)
 *    void OnOrgDeath(genome_t & genome)
 */

#ifndef MABE_GENOME_WRAPPER_H
#define MABE_GENOME_WRAPPER_H

#include <string>
#include <utility>

#include "config/config.h"
#include "meta/meta.h"

// Macros to take a variable list of macro arguments and wrap them all in declval.  These mock
// instances of the types will then be used in a function call in a decltype to determine if
// that specific type of call is legal.
//#define MABE_TYPE_TO_VAL(TYPE) std::declval< std::decay<TYPE> >()
//#define MABE_TYPE_TO_VAL(TYPE) *((std::decay<TYPE> *) nullptr)
#define MABE_TYPE_TO_VAL(TYPE) std::declval< TYPE >()
#define MABE_TYPES_TO_VALS(...) EMP_WRAP_ARGS(MABE_TYPE_TO_VAL, EMP_POP_ARG(__VA_ARGS__))

// Macro to dynamically call a function either in the wrapped type (if it exists)
// or return the default provided (otherwise).  The first two arguments are the
// function name and its default return.  The remaining arguments in the ... must
// be the return type (required) and all argument types (if any exist)
#define MABE_GENOME_TEST_FUN(NAME, DEFAULT, ...)                              \
    template <typename T>                                                     \
    using return_t_ ## NAME =                                                 \
      EMP_IF( EMP_EQU( EMP_COUNT_ARGS(__VA_ARGS__), 1),                       \
        decltype( std::declval<T>().NAME() );,                                \
        decltype( std::declval<T>().NAME(MABE_TYPES_TO_VALS(__VA_ARGS__)) );  \
      )                                                                       \
    static constexpr bool HasFun_ ## NAME() {                                 \
      return emp::test_type<return_t_ ## NAME, GENOME_T>();                   \
    }                                                                         \
    template <typename... Ts>                                                 \
    EMP_GET_ARG(1, __VA_ARGS__) NAME(Ts &&... args) {                         \
      if constexpr (HasFun_ ## NAME()) {                                      \
        return GENOME_T::NAME( std::forward<Ts>(args)... );                   \
      }                                                                       \
      else { return DEFAULT; }                                                \
    }


namespace mabe {

  template <typename GENOME_T>
  class GenomeWrapper : public GENOME_T {
  private:
    using this_t = GenomeWrapper<GENOME_T>;

    static emp::Config empty_config;

  public:
    // Create a set of functions to determine if a memeber exists on GENOME_T in the form of
    // constexpr bool has_fun_X()
    MABE_GENOME_TEST_FUN(GetClassName, "NoName", std::string);
    MABE_GENOME_TEST_FUN(GetConfig, empty_config, emp::Config);
    MABE_GENOME_TEST_FUN(Randomize, false, bool, emp::Random &);
    MABE_GENOME_TEST_FUN(Print, false, bool);
    MABE_GENOME_TEST_FUN(OnBeforeRepro, false, bool);              // Genome about to be reproduced.
    MABE_GENOME_TEST_FUN(OnOffspringReady, false, bool, this_t &); // Genome offspring; arg is parent genome
    MABE_GENOME_TEST_FUN(OnInjectReady, false, bool);              // Genome about to be injected.
    MABE_GENOME_TEST_FUN(OnBeforePlacement, false, bool);          // Genome about to be placed
    MABE_GENOME_TEST_FUN(OnPlacement, false, bool);                // Genome just placed.
    MABE_GENOME_TEST_FUN(OnOrgDeath, false, bool);                 // Genome about to die.
  };

}

#endif

