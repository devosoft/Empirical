#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Ptr.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");
  
  // Test default constructor.
  emp::Ptr<int> ptr1;
  ptr1.New();
  *ptr1 = 5;
  emp_assert(*ptr1 == 5);
  ptr1.Delete();
  
  // Test pointer constructor
  emp::Ptr<int> ptr2(new int);
  *ptr2 = 10;
  emp_assert(*ptr2 == 10);
  ptr2.Delete();

  // Test non-pointer object constructor
  int base_val = 15;
  emp::Ptr<int> ptr3(base_val);
  emp_assert(*ptr3 == 15);
  base_val = 20;                 // Make sure pointed to value changes with original variable.
  emp_assert(*ptr3 == 20);

  // Test copy-constructor.
  emp::Ptr<int> ptr4(ptr3);
  emp_assert(*ptr4 == 20);
  *ptr4 = 25;                    // Change this new pointer...
  emp_assert(*ptr4 == 25);       // ...make sure it actually changed.
  emp_assert(*ptr3 == 25);       // ...make sure the other pointer reflects the change.
  emp_assert(base_val == 25);    // ...make sure the original variable changed.


  // Make sure pointer trackers are working.
  emp::PtrTracker<int> & tracker = emp::PtrTracker<int>::Get();
}
