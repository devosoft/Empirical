#include <iostream>
#include <vector>
#include "../../hardware/HardwareCPU_Evo.h"
#include "../../hardware/Instruction_Evo.h"
#include "../../hardware/InstLib.h"
#include "../../hardware/HardwareCallback.h"
#include "../../tools/errors.h"

void PrintStackTops(emp::HardwareCPU_Evo<> & cpu) {
  std::cout << "Stack TOPS: ";
  for (int i = 0; i < 8; i++) {
    std::cout << i << ":" << cpu.GetStack(i).Top() << "  ";
  }
  std::cout << std::endl;
}

int main()
{
  emp::InstLib<emp::HardwareCPU_Evo<>, emp::Instruction_Evo> inst_lib;
  inst_lib.LoadDefaults();
  emp::HardwareCPU_Evo<> cpu(inst_lib);

  std::vector<emp::Instruction_Evo> code;
  code.push_back(inst_lib["Inc"]);
  code.push_back(inst_lib["Nop-7"]);
  code.push_back(inst_lib["Inc"]);
  code.push_back(inst_lib["Nop-0"]);
  code.push_back(inst_lib["Push-Zero"]);
  code.push_back(inst_lib["Nop-7"]);
  code.push_back(inst_lib["Bookmark"]);
  code.push_back(inst_lib["Inc"]);
  code.push_back(inst_lib["Nop-1"]);
  code.push_back(inst_lib["Jump"]);
  code.push_back(inst_lib["Inc"]);
  code.push_back(inst_lib["Nop-2"]);

  cpu.LoadMemory(code);

  PrintStackTops(cpu);

  for (int i = 0; i < 15; i++) {
    cpu.SingleProcess();
    PrintStackTops(cpu);
  }
  
//   const emp::Instruction_Evo & inst = inst_lib["Nop-5"];
//   std::cout << "Inst info: "
//             << "id=" << inst.GetID() << " "
//             << "arg=" << inst.GetArgValue()-1 << " "
//             << "ptr=" << (&inst)
//             << std::endl;
  
//   std::cout << "Lookup info: "
//             << "name=" << inst_lib.GetName(inst) << " "
//             << "symbol=" << inst_lib.GetShortName(inst) << " "
//             << "cycle=" << inst_lib.GetCycleCost(inst) << " "
//               << "id=" << inst_lib.GetID(inst)
//             << std::endl << std::endl;
}
