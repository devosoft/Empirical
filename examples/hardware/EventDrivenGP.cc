
#include <iostream>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  emp::Ptr<emp::Random> random = new emp::Random(2);

  emp::EventDrivenGP cpu(random);
  std::cout << "-- Print empty state. --" << std::endl;
  cpu.PrintState();
  std::cout << "-- Print empty program. --" << std::endl;
  cpu.PrintProgram(); std::cout << std::endl;
  /*
  emp::EventDrivenGP::Function fun0;
  fun0.affinity.Randomize(*random, 0.5);
  cpu.PushFunction(fun0);
  cpu.PushInst("Nop");
  cpu.PushInst("Return");
  cpu.PushInst("Nop");
  cpu.PushInst("Call", 0, 0, 0, emp::BitSet<4>(*random, 0.5));
  cpu.PushInst("Nop");

  emp::EventDrivenGP::Function fun1;
  fun1.affinity.Randomize(*random, 0.5);
  cpu.PushFunction(fun1);
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Call", 0, 0, 0, emp::BitSet<4>(*random, 0.5));
  cpu.PushInst("Nop");

  emp::EventDrivenGP::Function fun2;
  fun2.affinity.Randomize(*random, 0.5);
  cpu.PushFunction(fun2);
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Call", 0, 0, 0, emp::BitSet<4>(*random, 0.5));
  cpu.PushInst("Nop");

  emp::EventDrivenGP::Function fun3;
  fun3.affinity.Randomize(*random, 0.5);
  cpu.PushFunction(fun3);
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Return");
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  cpu.PushInst("Nop");
  */


  cpu.PushInst("SetMem", 1, 99);
  cpu.PushInst("CopyMem", 1, 2);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("SwapMem", 1, 2);

  std::cout << "-- Print simple program. --" << std::endl;
  cpu.PrintProgram(); std::cout << std::endl;

  // Create some random events.
  using event_t = emp::EventDrivenGP::Event;

  event_t e1({{1,2.0},{2,4.0},{5,10.0}}, "message", emp::BitSet<4>(*random, 0.5));
  event_t e2({{1,0.5},{4,2.0},{10,5.0}}, "message", emp::BitSet<4>(*random, 0.5));
  event_t e3({{1,0.5},{4,2.0},{10,5.0}}, "message", emp::BitSet<4>(*random, 0.5));
  cpu.QueueEvent(e1);
  cpu.QueueEvent(e2);
  cpu.QueueEvent(e3);
  // Run program
  std::cout << "-- Running simple program. --" << std::endl;
  cpu.PrintState();
  for (size_t i = 0; i < 5; i++) {
    std::cout << "==== Update: " << i << " ====" << std::endl;
    cpu.Process(1);
    cpu.PrintState();
  }
  return 0;
}
