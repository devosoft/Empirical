#include <iostream>

#include "../../tools/assert.h"

int main()
{
  // Asserts are tricky to test.  Here are a bunch that should pass.
  emp_assert(true);
  emp_assert(100);
  emp_assert(23 < 24);
  emp_assert((14 < 13)?0:1);

  return 0;
}
