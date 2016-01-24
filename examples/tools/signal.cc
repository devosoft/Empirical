//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Signal

#include <iostream>
#include "../../tools/signal.h"

void PrintInt(int i) {
  std::cout << "[" << i << "]" << std::endl;
}

void PrintVoid() {
  std::cout << "***" << std::endl;
}

int main()
{
  // A simple signal that sends an int.
  emp::Signal<int> test_signal("test");
  test_signal.AddAction(PrintInt);
  test_signal.Trigger(12);
  test_signal.Trigger(-1);

  // Signals dont need to take arguments or have names.
  emp::Signal<> test_signal2;
  test_signal2.AddAction(PrintVoid);
  test_signal2.Trigger();
}
