//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some examples code for using emp::TimeQueue

#include <iostream>
#include "tools/TimeQueue.h"

int main()
{
  emp::TimeQueue time_queue(100.0);

  time_queue.Insert(0, 130.0);
  time_queue.Insert(1, 150.0);
  time_queue.Insert(2, 140.0);
  time_queue.Insert(3, 120.0);
  time_queue.Insert(4, 260.0);

  // Print the first item; advance time to 120.
  std::cout << time_queue.Next() << std::endl;
  time_queue.Insert(5, 110.0);  // Should be inserted at 120+110 = 230 (before 4).
  
  
  while (time_queue.GetSize()) {
    std::cout << time_queue.Next() << std::endl;
  }

  std::cout << "End time = " << time_queue.GetTime() << std::endl;
}
