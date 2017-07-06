
#include <iostream>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  emp::Ptr<emp::Random> random = new emp::Random(2);
  using event_t = typename emp::EventDrivenGP::event_t;

  emp::EventDrivenGP cpu(random);
  cpu.GetEventLib()->RegisterDispatchFun("Message",
      [](emp::EventDrivenGP & hw, const event_t & event){
        std::cout << "Trigger message event(1)!" << std::endl;
        hw.PrintEvent(event); std::cout << std::endl;
      });
  cpu.GetEventLib()->RegisterDispatchFun("Message",
      [](emp::EventDrivenGP & hw, const event_t & event) {
        std::cout << "Trigger message event(2)!" << std::endl;
        hw.PrintEvent(event); std::cout << std::endl;
      });

  std::cout << "-- Print empty state. --" << std::endl;
  cpu.PrintState();
  std::cout << "-- Print empty program. --" << std::endl;
  cpu.PrintProgram(); std::cout << std::endl;

  cpu.PushInst("SetMem", 1, 99);
  cpu.PushInst("CopyMem", 1, 2);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("SwapMem", 1, 2);
  cpu.PushInst("Output", 1, 1);
  cpu.PushInst("Output", 2, 2);
  cpu.PushInst("SendMsg");
  cpu.PushInst("BroadcastMsg");

  std::cout << "-- Print simple program. --" << std::endl;
  cpu.PrintProgram(); std::cout << std::endl;

  // Run program
  std::cout << "-- Running simple program. --" << std::endl;
  cpu.PrintState();
  for (size_t i = 0; i < 15; i++) {
    std::cout << "==== Update: " << i << " ====" << std::endl;
    cpu.Process(1);
    cpu.PrintState();
  }
  return 0;
}
