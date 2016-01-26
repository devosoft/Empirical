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

void MultInt(int i, int j) {
  std::cout << "[" << i*j << "]" << std::endl;
}


int main()
{
  // A simple signal that sends an int.
  emp::Signal<int> test_sig("test");
  test_sig.AddAction(PrintInt);
  test_sig.AddAction(PrintVoid);
  test_sig.AddAction([](int x){std::cout<<"---:"<<x<<std::endl<<std::endl;});
  test_sig.Trigger(12);
  test_sig.Trigger(-1);

  // Signals dont need to take arguments or have names.
  emp::Signal<> test_sig2;
  test_sig2.AddAction(PrintVoid);
  test_sig2.Trigger();


  // Actions can be turned into named objects as well.
  int total=0;
  emp::Action<int> act1([&total](int inc){total+=inc;}, "sum");
  emp::Action<int> act2(PrintInt, "iprint");
  emp::Action<int,int>(MultInt, "mint");

  emp::Signal<int> test_sig3("test3");
  test_sig3.AddAction(act1);

  emp::LinkSignal("test3", "iprint");
  emp::LinkSignal("test3", "iprint");
  test_sig3.AddAction(act2);

  test_sig3.Trigger(10);
  test_sig3.Trigger(20);

  test_sig.Trigger(total);
}
