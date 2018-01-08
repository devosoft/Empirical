//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../hardware/AvidaGPOthello.h"
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

/*  cpu.PushInst("SetReg", 0, 43);
  cpu.PushInst("SetReg", 1, 20);
  cpu.PushInst("SetReg", 5, 29);
  cpu.PushInst("SetReg", 6, 34);
  cpu.PushInst("SetReg", 7, 19);
  cpu.PushInst("SetBoard", 0);
  cpu.PushInst("GetSquareCurr", 0, 2); // Should be 0 in reg 2
  cpu.PushInst("GetValidAbove", 0, 3);
  cpu.PushInst("GetValidBelow", 1, 4);
  cpu.PushInst("GetValidAbove", 1, 3);
  cpu.PushInst("GetValidBelow", 0, 4);

  cpu.PushInst("GetValidLeft", 5, 3);
  cpu.PushInst("GetValidRight", 6, 4);
  cpu.PushInst("GetValidLeft", 6, 3);
  cpu.PushInst("GetValidRight", 5, 4);
  cpu.PushInst("GetValidLR", 7, 3);
  cpu.PushInst("GetValidLR", 6, 4);*/


 /* // Get Input and set it in memory
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);
  cpu.PushInst("While", 1, 1 );
  cpu.PushInst("Input", 2, 3);
  cpu.PushInst("SetMem", 0, 2, 3);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0);

  // Check each spot on board for valid move 9
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);
  cpu.PushInst("SetReg", 4, 0);
  cpu.PushInst("While", 1, 1 ); // START WHILE 1
  cpu.PushInst("GetMem", 0, 2, 3);
  // Check if space is empty
  cpu.PushInst("TestNEqu", 3, 4, 5);
  cpu.PushInst("If", 5, 2);
  cpu.PushInst("Inc", 2);
  cpu.PushInst("Break", 0);
  cpu.PushInst("Scope", 2); //TODO Fix scopes
  // Check if there is a valid move above current position
  cpu.PushInst("SetReg", 6, 0);
  cpu.PushInst("SetReg", 7, 0);
  cpu.PushInst("Dec", 7);
  cpu.PushInst("SetReg", 8, 8);
  cpu.PushInst("Sub", 2, 8, 9);
  cpu.PushInst("TestLess", 9, 4, 10);
  cpu.PushInst("Not", 10);
  cpu.PushInst("While", 10, 2); // START WHILE 2
  cpu.PushInst("Sub", 9, 8, 9);
  cpu.PushInst("TestLess", 9, 4, 10);
  cpu.PushInst("Not", 10);
  cpu.PushInst("Scope", 2); // END WHILE 2

  cpu.PushInst("Inc", 2);
  cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0); // END WHILE 1*/

  cpu.PushInst("SetBoard", 0);
  cpu.PushInst("SetReg", 0, 64);
  cpu.PushInst("SetReg", 2, 0);
  cpu.PushInst("SetReg", 1, 1);

  cpu.PushInst("While", 1, 1 );
    cpu.PushInst("GetValidAbove", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidBelow", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLeft", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidRight", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidUL", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidUR", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLL", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("GetValidLR", 2, 3);
    cpu.PushInst("If", 3, 2);
      cpu.PushInst("SetReg", 1, 0);
      cpu.PushInst("Break", 0);
    cpu.PushInst("Scope", 2);
    cpu.PushInst("Inc", 2);
    cpu.PushInst("TestNEqu", 2, 0, 1);
  cpu.PushInst("Scope", 0);
  cpu.PushInst("Output", 2, 0);



  /*for (size_t i = 0; i < 100; i++) {
    cpu.Process(1);
    Print(cpu);
  }*/

  cpu.PrintGenome();
//  cpu.ResetHardware();
  cpu.Trace(556);

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
