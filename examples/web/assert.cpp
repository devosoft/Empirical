/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file assert.cpp
 */

#include "emp/base/assert.hpp"
#include "emp/web/web.hpp"

int main() {

  int x{ 42 };

  emp_assert(x > 1, "This assert passes in debug mode!", x);

  emp_assert(x < 1, "This assert fails in debug mode!", x);


}
