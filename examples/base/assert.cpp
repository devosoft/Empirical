//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code demonstrating use of the error system.

#include "emp/base/assert.hpp"

int main() {

  int x{ 42 };

  emp_assert(x > 1, "This assert passes in debug mode!", x);

  emp_assert(x < 1, "This assert fails in debug mode!", x);

}
