
#include <iostream>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  // Some convenient type aliases:
  using event_t = typename emp::EventDrivenGP::event_t;
  using affinity_t = emp::BitSet<8>;


  // Setup random number generator.
  emp::Ptr<emp::Random> random = new emp::Random(2);

  emp::EventDrivenGP cpu(random);

  // Register some example event dispatchers.
  // cpu.GetEventLib()->RegisterDispatchFun("Message",
  //     [](emp::EventDrivenGP & hw, const event_t & event){
  //       std::cout << "----------->" << std::endl;
  //       std::cout << "From dispatcher 1: Message event!" << std::endl;
  //       hw.PrintEvent(event); std::cout << std::endl;
  //       std::cout << "<----------" << std::endl;
  //     });
  // cpu.GetEventLib()->RegisterDispatchFun("Message",
  //     [](emp::EventDrivenGP & hw, const event_t & event) {
  //       std::cout << "----------->" << std::endl;
  //       std::cout << "From dispatcher 2: Message event!" << std::endl;
  //       hw.PrintEvent(event); std::cout << std::endl;
  //       std::cout << "<----------" << std::endl;
  //     });

  // Making a table of bit set values to make affinity assignment easier.
  emp::vector<affinity_t> bitsets(256);
  for (size_t i = 0; i < bitsets.size(); ++i) {
    bitsets[i].SetByte(0, (uint8_t)i);
  }

  emp::EventDrivenGP::Function fun0(bitsets[0]);
  emp::EventDrivenGP::Function fun1(bitsets[255]);
  emp::EventDrivenGP::Function fun2(bitsets[219]);
  emp::EventDrivenGP::Function fun3(bitsets[240]);

  cpu.PushFunction(fun0);
  cpu.PushInst("Inc", 0);
  cpu.PushInst("Call", 0, 0, 0, bitsets[240]);
  cpu.PushFunction(fun1);
  cpu.PushInst("Pull", 0, 0);
  cpu.PushInst("Inc", 0);
  cpu.PushInst("Commit", 0);
  cpu.PushFunction(fun2);
  cpu.PushInst("Nop");
  cpu.PushInst("Call", 0, 0, 0, bitsets[255]);
  cpu.PushInst("Nop");
  cpu.PushFunction(fun3);
  cpu.PushInst("SetMem", 0, 10);
  cpu.PushInst("Countdown", 0);
  cpu.PushInst("Pull", 0, 1);
  cpu.PushInst("Output", 1, 0);
  cpu.PushInst("BroadcastMsg", 0, 0, 0, bitsets[1]);
  cpu.PushInst("Close");

  std::cout << "-- Print CPU's program. --" << std::endl;
  cpu.PrintProgram(); std::cout << std::endl;

  // Run program
  std::cout << "-- Running simple program. --" << std::endl;
  cpu.PrintState();
  for (size_t i = 0; i < 100; i++) {
    std::cout << "==== Update: " << i << " ====" << std::endl;
    // Every 10 updates, Queue up an event.
    if (i % 10 == 0)
      cpu.QueueEvent("Message", bitsets[219], {{0,1.0},{1,2.0},{2,4.0}});
    cpu.Process(1);
    cpu.PrintState();
  }
  return 0;
}
