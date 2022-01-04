/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file errors.cpp
 *  @brief Some examples code demonstrating use of the error system.
 */

#include <iostream>

#include "emp/base/errors.hpp"

int main()
{
  std::cout << "Testing" << std::endl;

  emp::TriggerExcept("test_fail", "The test failed.  *sob*");
  emp::TriggerExcept("test_fail2", "The second test failed too.  But it's not quite as aweful.", false);
  emp::TriggerExcept("test_fail2", "The third test is just test 2 again, but worse", true);

  std::cout << emp::CountExcepts() << " exceptions found." << std::endl;
  auto except = emp::PopExcept("test_fail2");
  std::cout << "Poped a test_fail2; now " << emp::CountExcepts() << " exceptions found." << std::endl;

  std::cout << "test_fail2: " << except.desc << std::endl;

  std::cout << "Do we still have the other version of the test? "
            << emp::HasExcept("test_fail2") << std::endl;
  std::cout << "Do we have a test_fail3? "
            << emp::HasExcept("test_fail3") << std::endl;

  emp::ClearExcepts();
  std::cout << "And after a clear, exceptions cout = " << emp::CountExcepts() << std::endl;
}
