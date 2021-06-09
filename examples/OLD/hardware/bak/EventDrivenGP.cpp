// ------- DEPRECATED -------
#include <iostream>
#include <fstream>
#include "hardware/EventDrivenGP.h"
#include "base/Ptr.h"
#include "tools/Random.h"
#include "tools/BitSet.h"

int main() {
  std::cout << "Testing EventDrivenGP." << '\n';

  // Some convenient type aliases:
  using inst_t = typename emp::EventDrivenGP::inst_t;
  using inst_lib_t = typename emp::EventDrivenGP::inst_lib_t;
  using event_t = typename emp::EventDrivenGP::event_t;
  using event_lib_t = typename emp::EventDrivenGP::event_lib_t;
  using affinity_t = typename emp::EventDrivenGP::affinity_t;

  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  // Setup random number generator.
  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>();
  emp::Ptr<inst_lib_t> inst_lib = emp::NewPtr<inst_lib_t>(*emp::EventDrivenGP::DefaultInstLib());
  emp::Ptr<event_lib_t> event_lib = emp::NewPtr<event_lib_t>(*emp::EventDrivenGP::DefaultEventLib());

  emp::EventDrivenGP cpu0(inst_lib, event_lib, random);
  emp::EventDrivenGP cpu1(cpu0);

  // Test Load function.
  std::ifstream prog_fstream("EventDrivenGP_1.gp");
  cpu0.Load(prog_fstream);
  std::ifstream prog2_fstream("EventDrivenGP_2.gp");
  cpu1.Load(prog2_fstream);

  std::cout << "=====================================" << '\n';
  std::cout << "CPU 0's PROGRAM: " << '\n';
  std::cout << "Abbreviated version:" << '\n';
  cpu0.PrintProgram();
  std::cout << "Full version:" << '\n';
  cpu0.PrintProgramFull();
  std::cout << "=====================================" << '\n';

  std::cout << "=====================================" << '\n';
  std::cout << "CPU 1's PROGRAM: " << '\n';
  std::cout << "Abbreviated version:" << '\n';
  cpu1.PrintProgram();
  std::cout << "Full version:" << '\n';
  cpu1.PrintProgramFull();
  std::cout << "=====================================" << '\n';

  // Configure hardware.
  cpu0.SetMinBindThresh(1.0);
  cpu0.SetMaxCores(7);
  cpu0.SetMaxCallDepth(64);
  cpu0.SetDefaultMemValue(1.0);
  cpu1.SetMaxCores(16);

  cpu0.TriggerEvent("Message"); // Trigger event w/no dispatch function (should do nothing).
  cpu0.QueueEvent("Message", affinity_t(), {{42, 48}}, {"send"});
  cpu0.CallFunction(0);
  // Spawn a bunch of cores (1 too many).
  for (size_t i = 0; i < cpu0.GetMaxCores(); ++i) {
    cpu0.SpawnCore(random->GetUInt(cpu0.GetProgram().GetSize()));
  }
  cpu0.PrintState();

  // Run for a bit.
  cpu0.Process(100);
  cpu0.PrintState();

  // Kill some cores (reduce max cores)
  cpu0.SetMaxCores(2);
  cpu0.PrintState();

  // Clean up dynamically allocated memory.
  inst_lib.Delete();
  event_lib.Delete();
  random.Delete();
  return 0;
}
