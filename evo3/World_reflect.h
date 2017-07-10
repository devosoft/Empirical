//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Handle reflection on organisms to setup reasonable defaults in the World template class.

#ifndef EMP_EVO_WORLD_REFLECT_H
#define EMP_EVO_WORLD_REFLECT_H

#include <functional>
#include <type_traits>

#include "../base/assert.h"
#include "../meta/reflection.h"

namespace emp {

  namespace {

    // Setup Fitness reflection.
    // 1. If an organism has a "GetFitness()" member function, use it!
    // 2. If an organim can be cast to double, use it!
    // 3. Start with a fitness function that throws an assert indicating function must be set.

    using std::declval;

    template <typename WORLD, typename ORG>
    void SetDefaultFitFun_impl(WORLD & world, bool_decoy<decltype( declval<ORG>().GetFitness() )>) {
      world.SetFitFun( [](ORG & org){ return (double) org.GetFitness(); } );
    }

    template <typename WORLD, typename ORG>
    void SetDefaultFitFun_impl(WORLD & world, int_decoy<decltype( (double) declval<ORG>() )>) {
      world.SetFitFun( [](ORG & org){ return (double) org; } );
    }

    template <typename WORLD, typename ORG>
    void SetDefaultFitFun_impl(WORLD & world, ... ) {
      world.SetFitFun( [](ORG & org){
        emp_assert(false, "No default fitness function available");
        return 0.0;
      } );
    }

  }

  template <typename WORLD, typename ORG>
  void SetDefaultFitFun(WORLD & world) { SetDefaultFitFun_impl<WORLD, ORG>(world, true); }

  namespace {
    // Setup Mutation function
    // 1. DoMutations(random)
    // 2. Empty, with assert.

    template <typename WORLD, typename ORG>
    void SetDefaultMutFun_impl(WORLD & world, bool_decoy<decltype( declval<ORG>().DoMutations(Random()) )>) {
      world.SetMutFun( [](ORG & org, Random & random) {
        return (double) org.DoMutations(random)();
      } );
    }

    template <typename WORLD, typename ORG>
    void SetDefaultMutFun_impl(WORLD & world, ... ) {
      world.SetMutFun( [](ORG & org, Random & random){
        emp_assert(false, "No default DoMutations available");
        return 0.0;
      } );
    }

  }

  template <typename WORLD, typename ORG>
  void SetDefaultMutFun(WORLD & world) { SetDefaultMutFun_impl<WORLD, ORG>(world, true); }


  namespace {
    // Setup Print function
    // 1. Org Print()
    // 2. Proper operator<<
    // 3. Assert
    // @CAO: Also try emp::to_string ??

    template <typename WORLD, typename ORG>
    void SetDefaultPrintFun_impl(WORLD & world, bool_decoy<decltype( declval<ORG>().Print(std::cout) )>) {
      world.SetPrintFun( [](ORG & org, std::ostream & os){ org.Print(os); } );
    }

    template <typename WORLD, typename ORG>
    void SetDefaultPrintFun_impl(WORLD & world, int_decoy<decltype( std::cout << declval<ORG>() )>) {
      world.SetPrintFun( [](ORG & org, std::ostream & os){ os << org; } );
    }

    template <typename WORLD, typename ORG>
    void SetDefaultPrintFun_impl(WORLD & world, ... ) {
      world.SetPrintFun( [](ORG & org, std::ostream & os){
        emp_assert(false, "No default Print function available");
      } );
    }

  }

  template <typename WORLD, typename ORG>
  void SetDefaultPrintFun(WORLD & world) { SetDefaultPrintFun_impl<WORLD, ORG>(world, true); }


  namespace {
    // Setup genome type identification
    template <typename ORG>
    auto Org2Genome( bool_decoy< decltype( declval<ORG>().GetGenome() ) >)
      -> std::decay_t< decltype( declval<ORG>().GetGenome() ) >;

    template <typename ORG>
    ORG Org2Genome( ... );
  }

  template <typename ORG>
  using find_genome_t = decltype( Org2Genome<ORG>() );


  namespace {
    // Setup Org -> Genome function
    // 1. GetGenome member function
    // 2. Return org AS genome.

    template <typename WORLD, typename ORG>
    void SetOrgGetGenome_impl(WORLD & world, bool_decoy<decltype( declval<ORG>().GetGenome() )>) {
      world.SetGetGenomeFun( [](ORG & org) -> const auto & { return org.GetGenome(); } );
    }

    template <typename WORLD, typename ORG>
    void SetOrgGetGenome_impl(WORLD & world, ... ) {
      world.SetGetGenomeFun( [](ORG & org) -> const ORG & { return org; } );
    }

  }

  template <typename WORLD, typename ORG>
  void SetDefaultGetGenomeFun(WORLD & world) { SetOrgGetGenome_impl<WORLD, ORG>(world, true); }

}

#endif
