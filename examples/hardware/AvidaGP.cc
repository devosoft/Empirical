//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../hardware/AvidaGP.h"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "regs[" << i << "] = " << cpu.GetReg(i) << "  ";
  }
  std::cout << std::endl;
}

int main()
{
  std::cout << "Test." << std::endl;

  emp::AvidaGP cpu;
  Print(cpu);
  cpu.PushInst( emp::AvidaGP::Inst::Inc, 1);
  cpu.SingleProcess();
  Print(cpu);

  return 0;
}

