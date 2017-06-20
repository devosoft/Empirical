//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../hardware2/AvidaGP.h"
#include "../../hardware2/InstLib.h"
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
  Print(cpu);
  cpu.PushInst( emp::AvidaGP::InstID::Countdown, 8, 4 );
  cpu.PushInst( emp::AvidaGP::InstID::Mult, 6, 2, 6 );
  cpu.PushInst( emp::AvidaGP::InstID::Scope, 0 );

  cpu.PushInst( emp::AvidaGP::InstID::Inc, 1 );
  cpu.PushInst( emp::AvidaGP::InstID::Inc, 1 );
  cpu.PushInst( emp::AvidaGP::InstID::Mult, 1, 2, 3 );
  cpu.PushInst( emp::AvidaGP::InstID::Add, 3, 4, 5 );

  for (int i = 0; i < 43; i++) cpu.PushRandom(random);

  for (size_t i = 0; i < 100; i++) {
    cpu.Process(1);
    Print(cpu);
  }

  cpu.PrintGenome();
  cpu.ResetHardware();
  cpu.Trace(200);

  // Run a ton of organisms.
  for (int t = 0; t < 100000; t++) {
    //if (t % 10000 == 0) std::cout << t << std::endl;
    cpu.Reset();
    for (int i = 0; i < 100; i++) cpu.PushRandom(random);
    cpu.Process(200);
  }

  return 0;
}
