//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Handle reflection on organisms to setup reasonable defaults in the World template class.

#ifndef EMP_EVO_WORLD_H
#define EMP_EVO_WORLD_H

namespace emp {

  // Setup Fitness reflection.
  // 1. If an organism has a "GetFitness()" member function, use it!
  // 2. If an organim can be cast to double, use it!
  // 3. Start with a fitness function that throws an assert indicating function must be set.


  // Setup Mutation function
  // 1. DoMutations(random)
  // 2. Empty, with assert.


  // Setup Print function
  // 1. Proper operator<<
  // 2. Print()
  // 3. Assert?  Or trivial default?

}

#endif
