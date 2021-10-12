//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Some examples code for using emp::TimeQueue

#include <iostream>
#include "emp/datastructs/TimeQueue.hpp"

int main()
{
  emp::TimeQueue time_queue;

  time_queue.Insert(0, 130.0);
  time_queue.Insert(1, 150.0);
  time_queue.Insert(2, 140.0);
  time_queue.Insert(3, 120.0);
  time_queue.Insert(4, 260.0);

  // Print the first item; advance time to 120.
  std::cout << time_queue.Next() << std::endl;
  time_queue.Insert(5, 230.0);

  while (time_queue.GetSize()) {
    std::cout << time_queue.Next() << std::endl;
  }

  std::cout << "End time = " << time_queue.GetTime() << std::endl;


  std::cout << "\nNow with strings:\n";

  emp::TimeQueue<std::string> time_queue2;

  time_queue2.Insert("Item 0", 1.3);
  time_queue2.Insert("Item 1", 1.5);
  time_queue2.Insert("Item 2", 1.4);
  time_queue2.Insert("Item 3", 1.2);
  time_queue2.Insert("Item 4", 102.6);

  // Print the first item; advance time to 120.
  std::cout << time_queue2.Next() << std::endl;
  time_queue2.Insert("Item 5", 2.3);


  while (time_queue2.GetSize()) {
    std::cout << time_queue2.Next() << std::endl;
  }

  std::cout << "End time = " << time_queue2.GetTime() << std::endl;
}
