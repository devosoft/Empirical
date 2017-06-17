//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Handle reflection on organisms to setup reasonable defaults in the World template class.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

#include <type_traits>

namespace emp {

  namespace {
    using fit_fun_t = std::function<double(ORG&)>;
  }

  // Setup Fitness reflection.
  // 1. If an organism has a "GetFitness()" member function, use it!
  // 2. If an organim can be cast to double, use it!
  // 3. Start with a fitness function that throws an assert indicating function must be set.

  template <typename ORG>
  void SetDefaultFitFun(World<ORG> & world, bool_decoy<decltype(ORG::GetFitness)) {
    world.SetFitFun( [](ORG & org){ return (double) org.GetFitness(); } );
  }

  template <typename ORG>
  void SetDefaultFitFun(World<ORG> & world, int_decoy<decltype( (double)(*(ORG*)nullptr) )) {
    world.SetFitFun( [](ORG & org){ return (double) org; } );
  }

  template <typename ORG>
  void SetDefaultFitFun(World<ORG> & world, ... )) {
    world.SetFitFun( [](ORG & org){
      emp_assert(false, "No default fitness function available"); return 0.0;
    } );
  }

  template <typename ORG>
  void SetDefaultFitFun(World<ORG> & world) { SetDefaultFitFun(world, true); }

  // Setup Mutation function
  // 1. DoMutations(random)
  // 2. Empty, with assert.


  // Setup Print function
  // 1. Proper operator<<
  // 2. Print()
  // 3. Assert?  Or trivial default?

}

#endif
