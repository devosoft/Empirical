#include <ostream>

#include "tools/ContiguousStreamBuf.h"

int main() {

  emp::ContiguousStreamBuf cs;
  std::ostream out(&cs);

  // Send 12 letters: 11 + \n.
  out << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  out << "calloVerflow" << std::endl;
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  for (size_t i = 0; i < 5; ++i) {
    out << "jsad;kfjsa;lkdfja;ksdjfksajdkjfjjjjasdf Verflow" << std::endl;
  }
  cs.Print();
  std::cout << std::endl;

  std::cout << "Reset" << std::endl;
  cs.Reset();
  cs.Print();
  std::cout << std::endl;

  out << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  return 0;

}
