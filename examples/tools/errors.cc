//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code demonstrating use of the error system.

#include <iostream>

#include "../../tools/errors.h"

int main()
{
  std::cout << "Testing" << std::endl;

  emp::TriggerFailure("test_fail", "The test failed.  *sob*");
  emp::TriggerFailure("test_fail2", "The second test failed too.  But it's not quite as aweful.", false);
  emp::TriggerFailure("test_fail2", "The third test is just test 2 again, but worse", true);

  std::cout << emp::CountFailures() << " failures found." << std::endl;
  auto failure = emp::PopFailure("test_fail2");
  std::cout << "Poped a test_fail2; now " << emp::CountFailures() << " failures found." << std::endl;

  std::cout << "test_fail2: " << failure.desc << std::endl;

  std::cout << "Do we still have the other version of the test? "
            << emp::HasFailure("test_fail2") << std::endl;
  std::cout << "Do we have a test_fail3? "
            << emp::HasFailure("test_fail3") << std::endl;
}
