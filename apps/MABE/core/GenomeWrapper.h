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

#define MABE_GENOME_TEST_FUN(NAME, RETURN, DEFAULT, ...)                        \
    template <typename T>                                                       \
    using return_t_ ## NAME = decltype( std::declval<T>().NAME(__VA_ARGS__) );  \
    static constexpr bool HasFun_ ## NAME() {                                   \
      return emp::test_type<return_t_ ## NAME, GENOME_T>();                     \
    }                                                                           \
    RETURN NAME() const {                                                       \
      if constexpr (HasFun_ ## NAME()) { return GENOME_T::NAME(); }             \
      else { return DEFAULT; }                                                  \
    }

namespace mabe {

  template <typename GENOME_T>
  class GenomeWrapper : public GENOME_T {
  public:
    // Create a set of functions to determine if a memeber exists on GENOME_T in the form of
    // constexpr bool has_fun_X()
    MABE_GENOME_TEST_FUN(GetClassName, std::string, "NoName");
    // MABE_GENOME_TEST_FUN(GetConfig);
    // MABE_GENOME_TEST_FUN(Randomize);
    // MABE_GENOME_TEST_FUN(Print);
    // MABE_GENOME_TEST_FUN(OnBeforeRepro);
    // MABE_GENOME_TEST_FUN(OnOffspringReady);
    // MABE_GENOME_TEST_FUN(OnInjectReady);
    // MABE_GENOME_TEST_FUN(OnBeforePlacement);
    // MABE_GENOME_TEST_FUN(OnPlacement);
    // MABE_GENOME_TEST_FUN(OnOrgDeath);
  public:
    // std::string GetClassName() const {
    //   if constexpr (HasFun_GetClassName()) { return GENOME_T::GetClassName(); }
    //   else { return std::string("NoName"); }
    // }
  };

}

#endif

