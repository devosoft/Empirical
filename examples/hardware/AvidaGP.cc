//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../hardware/AvidaGP.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << std::endl;
}

int main()
{
  std::cout << "Test." << std::endl;

  emp::AvidaGP cpu;
  Print(cpu);
  cpu.PushInst( emp::AvidaGP::Inst::Inc, 1);
  cpu.PushInst( emp::AvidaGP::Inst::Inc, 1);
  cpu.PushInst( emp::AvidaGP::Inst::Mult, 1, 2, 3);
  cpu.PushInst( emp::AvidaGP::Inst::Add, 3, 4, 5);
  cpu.Process(4);
  Print(cpu);
  cpu.Process(4);
  Print(cpu);

  return 0;
}
