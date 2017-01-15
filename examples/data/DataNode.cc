//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataNode demonstrating how to track different types of data.

#include <iostream>

#include "../../data/DataNode.h"

int main()
{
  emp::DataNode<int, emp::DataCurrent, emp::DataStats> data;
  data.Add(27, 28, 29);

  std::cout << "=> Added 27, 28, and 29" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.Add(32);
  std::cout << "\n=> Added 32" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;


  data.Reset();
  std::cout << "\n=> Reset!" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.Add(100,200,300,400,500);
  std::cout << "\nAdded 100,200,300,400,500" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;
}
