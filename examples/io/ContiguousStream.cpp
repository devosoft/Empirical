/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ContiguousStream.cpp
 */

#include <ostream>

#include "emp/io/ContiguousStream.hpp"

int main() {

  emp::ContiguousStream cs;

  // Send 12 letters: 11 + \n.
  cs << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  cs << "calloVerflow" << std::endl;
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  for (size_t i = 0; i < 5; ++i) {
    cs << "jsad;kfjsa;lkdfja;ksdjfksajdkjfjjjjasdf Verflow" << std::endl;
  }
  cs.Print();
  std::cout << std::endl;

  std::cout << "Reset" << std::endl;
  cs.Reset();
  cs.Print();
  std::cout << std::endl;

  cs << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  return 0;

}
