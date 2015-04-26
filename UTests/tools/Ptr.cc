#include <iostream>
#include <vector>

// #define TDEBUG

#define EMP_TRACK_MEM

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Ptr.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");
  
  emp::PtrTracker<char>::Get().SetVerbose(verbose);
  emp::PtrTracker<int>::Get().SetVerbose(verbose);

  // Test default constructor.
  emp::Ptr<int> ptr1;
  ptr1.New();
  *ptr1 = 5;
  emp_assert(*ptr1 == 5);
  ptr1.Delete();
  
  // Test pointer constructor
  int * temp_int = new int;
  emp::Ptr<int> ptr2(temp_int);
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

  if (verbose) {
    std::cout << "Basic pointer operations passed tests." << std::endl;
  }

  // -- Test count tracking on emp::Ptr --
  // A bit of an odd set of test... we need to create and destory pointers to make sure
  // that all of the counts are correct, so we're going to use arrays of pointers to them.

  std::vector<emp::Ptr<char> *> ptr_set(10);
  ptr_set[0] = new emp::Ptr<char>;
  ptr_set[0]->New(42);
  for (int i = 1; i < 10; i++) ptr_set[i] = new emp::Ptr<char>(*(ptr_set[0]));

  // Do we have a proper count of 10?
  emp_assert(ptr_set[0]->DebugGetCount() == 10);
  ptr_set[1]->New(91);
  emp_assert(ptr_set[0]->DebugGetCount() == 9);
  *(ptr_set[2]) = *(ptr_set[1]);
  emp_assert(ptr_set[0]->DebugGetCount() == 8);
  emp_assert(ptr_set[1]->DebugGetCount() == 2);

  ptr_set[3]->Delete();

  std::cout << ptr_set[0]->DebugGetCount() << std::endl;

  // @CAO Make sure we don't delete below 0
  // @CAO Make sure we don't delete below 1 if we own it
  // @CAO Make sure we only delete if you own it
  // @CAO Make sure not to delete twice!
  // @CAO Make sure we don't add (as owner) a pointer we already own

  // -- Do some direct tests on pointer trackers --

  int * real_ptr1 = new int(1);  // Count of 2 in tracker
  int * real_ptr2 = new int(2);  // Deleted in tracker
  int * real_ptr3 = new int(3);  // Unknown to tracker
  int * real_ptr4 = new int(4);  // Passively known to tracker (marked non-owner)
  emp::PtrTracker<int> & tracker = emp::PtrTracker<int>::Get();
  
  tracker.New(real_ptr1);
  tracker.Inc(real_ptr1);
  tracker.Inc(real_ptr1);
  tracker.Dec(real_ptr1);

  tracker.New(real_ptr2);
  tracker.MarkDeleted(real_ptr2);

  tracker.Old(real_ptr4);

  emp_assert(tracker.HasPtr(real_ptr1) == true);
  emp_assert(tracker.HasPtr(real_ptr2) == true);
  emp_assert(tracker.HasPtr(real_ptr3) == false);
  emp_assert(tracker.HasPtr(real_ptr4) == true);

  emp_assert(tracker.IsActive(real_ptr1) == true);
  emp_assert(tracker.IsActive(real_ptr2) == false);
  emp_assert(tracker.IsActive(real_ptr3) == false);
  emp_assert(tracker.IsActive(real_ptr4) == true);

  emp_assert(tracker.IsOwner(real_ptr1) == true);
  emp_assert(tracker.IsOwner(real_ptr2) == true);
  emp_assert(tracker.IsOwner(real_ptr3) == false);
  emp_assert(tracker.IsOwner(real_ptr4) == false);

  emp_assert(tracker.GetCount(real_ptr1) == 2);
  emp_assert(tracker.GetCount(real_ptr2) == 1);
  emp_assert(tracker.GetCount(real_ptr3) == 0);
  emp_assert(tracker.GetCount(real_ptr4) == 1);

  if (verbose) {
    std::cout << "Pointer trackers passed tests." << std::endl;
  }
}
