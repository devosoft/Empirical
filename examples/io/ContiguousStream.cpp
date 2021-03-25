#include <ostream>

#include "emp/io/ContiguousStream.hpp"

int main() {

  emp::ContiguousStream cs;

  // Send 12 letters: 11 + \n.
  cs << "Hello_World";
  cs.Print();
  std::cout << '\n';

  // Overflow the buffer
  cs << "calloVerflow" << '\n';
  cs.Print();
  std::cout << '\n';

  // Overflow the buffer
  for (size_t i = 0; i < 5; ++i) {
    cs << "jsad;kfjsa;lkdfja;ksdjfksajdkjfjjjjasdf Verflow" << '\n';
  }
  cs.Print();
  std::cout << '\n';

  std::cout << "Reset" << '\n';
  cs.Reset();
  cs.Print();
  std::cout << '\n';

  cs << "Hello_World";
  cs.Print();
  std::cout << '\n';

  return 0;

}
