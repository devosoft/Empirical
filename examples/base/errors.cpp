//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code demonstrating use of the error system.

#include <iostream>

#include "emp/base/errors.hpp"

int main()
{
  std::cout << "Testing" << '\n';

  emp::TriggerExcept("test_fail", "The test failed.  *sob*");
  emp::TriggerExcept("test_fail2", "The second test failed too.  But it's not quite as aweful.", false);
  emp::TriggerExcept("test_fail2", "The third test is just test 2 again, but worse", true);

  std::cout << emp::CountExcepts() << " exceptions found." << '\n';
  auto except = emp::PopExcept("test_fail2");
  std::cout << "Poped a test_fail2; now " << emp::CountExcepts() << " exceptions found." << '\n';

  std::cout << "test_fail2: " << except.desc << '\n';

  std::cout << "Do we still have the other version of the test? "
            << emp::HasExcept("test_fail2") << '\n';
  std::cout << "Do we have a test_fail3? "
            << emp::HasExcept("test_fail3") << '\n';

  emp::ClearExcepts();
  std::cout << "And after a clear, exceptions cout = " << emp::CountExcepts() << '\n';
}
