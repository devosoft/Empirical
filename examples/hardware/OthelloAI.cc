//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../hardware/AvidaGPMem.h"
#include "../../hardware/InstLib.h"
#include "../../tools/Random.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

int main()
{
  std::cout << "Test." << std::endl;

  // const auto & inst_lib = emp::AvidaGP::GetInstLib();
  emp::Random random;

  emp::AvidaGP cpu;

  for (int i = 0; i < 64; i++) {
    double input = 0;
    if (i == 27 || i == 36) input = 1;
    else if (i == 28 || i == 35) input = -1;
    cpu.SetInput(i, input);
  }

  // Get Input and set it in memory
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);
  cpu.PushInst("While", 1, 1 );
  cpu.PushInst("Input", 2, 3);
  cpu.PushInst("SetMem", 0, 2, 3);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0);

  // Find one of your own pieces, put pos in reg 5
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);
  cpu.PushInst("While", 1, 1 );
  cpu.PushInst("GetMem", 0, 2, 3);
  cpu.PushInst("SetReg", 4, 0);
  cpu.PushInst("TestEqu", 3, 1, 4);
  cpu.PushInst("If", 4, 2);
  cpu.PushInst("CopyVal", 2, 5);
  cpu.PushInst("SetReg", 1, 0);
  cpu.PushInst("Break", 0);
  cpu.PushInst("Scope", 2);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0);


  /*for (size_t i = 0; i < 100; i++) {
    cpu.Process(1);
    Print(cpu);
  }*/

  cpu.PrintGenome();
//  cpu.ResetHardware();
  cpu.Trace(555);

  // Run a ton of organisms.
  /*for (int t = 0; t < 1000000; t++) {
    //if (t % 10000 == 0) std::cout << t << std::endl;
    cpu.Reset();
    for (int i = 0; i < 100; i++) cpu.PushRandom(random);
    cpu.Process(200);
  }*/
  std::cout << "NEW AvidaGP" << std::endl;

  return 0;
}
