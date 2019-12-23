/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MemoryImage.cc
 *  @brief An example file for using MemoryIamges
 */

#include <iostream>

#include "data/MemoryImage.h"

int main()
{
  emp::MemoryArray<100> mem_a;
  emp::MemoryVector mem_v;

  std::cout << "At start, mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  // Testing an Int...

  std::cout << "\n-- Testing Int" << std::endl;

  mem_a.AddObject<int>(20);
  mem_v.AddObject<int>(20);

  std::cout << "Added int 20; mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  std::cout << "Int values refs: mem_a[0] == " << mem_a.GetRef<int>(0)
            << "  mem_v[0] == " << mem_v.GetRef<int>(0)
            << std::endl;

  std::cout << "Int values ptrs: mem_a[0] == " << *(mem_a.GetPtr<int>(0))
            << "  mem_v[0] == " << *(mem_v.GetPtr<int>(0))
            << std::endl;

  // Testing a Double...

  std::cout << "\n-- Testing Double" << std::endl;

  mem_a.AddObject<double>(3.14159);
  mem_v.AddObject<double>(3.14159);

  std::cout << "Added double 3.14159; mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  std::cout << "Double values refs: mem_a[4] == " << mem_a.GetRef<double>(4)
            << "  mem_v[4] == " << mem_v.GetRef<double>(4)
            << std::endl;

  std::cout << "Double values ptrs: mem_a[4] == " << *(mem_a.GetPtr<double>(4))
            << "  mem_v[4] == " << *(mem_v.GetPtr<double>(4))
            << std::endl;

  // Changing a Double...

 std::cout << "\n-- Testing CHANGING the double" << std::endl;

  mem_a.GetRef<double>(4) = 222.222;
  mem_v.GetRef<double>(4) = 222.222;

  std::cout << "Changed double to 222.222; mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  std::cout << "Double values refs: mem_a[4] == " << mem_a.GetRef<double>(4)
            << "  mem_v[4] == " << mem_v.GetRef<double>(4)
            << std::endl;

  std::cout << "Double values ptrs: mem_a[4] == " << *(mem_a.GetPtr<double>(4))
            << "  mem_v[4] == " << *(mem_v.GetPtr<double>(4))
            << std::endl;


  // Testing a SHORT std::string...

  std::cout << "\n-- Testing SHORT Strings" << std::endl;

  mem_a.AddObject<std::string>("string1");
  mem_v.AddObject<std::string>("string1");

  std::cout << "Added string 'string1'; mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  std::cout << "String values refs: mem_a[12] == " << mem_a.GetRef<std::string>(12)
            << "  mem_v[12] == " << mem_v.GetRef<std::string>(12)
            << std::endl;

  std::cout << "String values ptrs: mem_a[12] == " << *(mem_a.GetPtr<std::string>(12))
            << "  mem_v[12] == " << *(mem_v.GetPtr<std::string>(12))
            << std::endl;

  // Changing a String...

 std::cout << "\n-- Testing CHANGING the SHORT string" << std::endl;

  mem_a.GetRef<std::string>(12)[6] = '2';
  mem_v.GetRef<std::string>(12)[6] = '2';

  std::cout << "Changed std::string to 'string2' altering last char; mem_a.size() == " << mem_a.size()
            << "  mem_v.size() == " << mem_v.size()
            << std::endl;

  std::cout << "String values refs: mem_a[12] == " << mem_a.GetRef<std::string>(12)
            << "  mem_v[12] == " << mem_v.GetRef<std::string>(12)
            << std::endl;

  std::cout << "String values ptrs: mem_a[12] == " << *(mem_a.GetPtr<std::string>(12))
            << "  mem_v[12] == " << *(mem_v.GetPtr<std::string>(12))
            << std::endl;


  // Try copying the MemoryImage.
  std::cout << "\n-- Testing copying!" << std::endl;

  emp::MemoryArray<100> mem_a2(mem_a);
  emp::MemoryVector mem_v2(mem_v);

  std::cout << "Made a copy; mem_a2.size() == " << mem_a2.size()
            << "  mem_v2.size() == " << mem_v2.size()
            << std::endl;

  std::cout << "Double values refs: mem_a2[4] == " << mem_a2.GetRef<double>(4)
            << "  mem_v2[4] == " << mem_v2.GetRef<double>(4)
            << std::endl;

  std::cout << "Double values ptrs: mem_a2[4] == " << *(mem_a2.GetPtr<double>(4))
            << "  mem_v2[4] == " << *(mem_v2.GetPtr<double>(4))
            << std::endl;


  // Clean up the images...

  std::cout << "\n-- Cleaning up!" << std::endl;

  mem_a.resize(0);
  mem_v.resize(0);
  mem_a2.resize(0);
  mem_v2.resize(0);
}
