
#include <iostream>
#include "../../hardware/EventDrivenGP.h"
#include "../../tools/Random.h"

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  emp::Random random;

  emp::EventDrivenGP cpu;
  std::cout << "-- Print empty state. --" << std::endl;
  cpu.PrintState();
  std::cout << "-- Print empty program. --" << std::endl;
  cpu.PrintProgram();
  return 0;
}
