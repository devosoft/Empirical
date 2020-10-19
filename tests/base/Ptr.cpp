//#ifndef EMP_TRACK_MEM
//#define EMP_TRACK_MEM

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/Ptr.hpp"

#include <sstream>
#include <iostream>
#include <functional>

TEST_CASE("Test Ptr", "[base]")
{
	emp::SetPtrDebug();
	REQUIRE(emp::GetPtrDebug());

	int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	emp::PtrInfo ptrInfo(arr, 10);
	REQUIRE(ptrInfo.IsArray());
	REQUIRE(ptrInfo.IsActive());
	REQUIRE(ptrInfo.OK());

	int arr1[5] = {-4, -3, -2, -1, 0};
	auto & tracker = emp::PtrTracker::Get();
	tracker.NewArray(arr1, 5 * sizeof(int));
	REQUIRE(tracker.HasPtr(arr1));
	size_t arr1ID = tracker.GetCurID(arr1);
	REQUIRE(tracker.IsActiveID(arr1ID));

	emp::Ptr<int> arr1ptr(arr1, 5, false);
	REQUIRE(!arr1ptr.IsNull());

	// attempts to delete const array fails, error: "discards qualifiers"

	arr1ptr.NewArray(10);
	//REQUIRE(arr1ptr.DebugGetArrayBytes() == (10 * sizeof(int)));

#ifdef EMP_TRACK_MEM
	size_t arr1ptrID = tracker.GetCurID(arr1ptr.Raw());
	REQUIRE(tracker.IsActiveID(arr1ptrID));
#endif
	arr1ptr.DeleteArray();
#ifdef EMP_TRACK_MEM
	REQUIRE(!tracker.IsActiveID(arr1ptrID));
#endif

	tracker.MarkDeleted(arr1ID);
	REQUIRE(!tracker.IsActiveID(arr1ID));

	int num = 123;
	int* num_ptr = &num;
	emp::Ptr<int> numPtr(num_ptr);
	emp::Ptr<int> numPtr2(num_ptr);
	REQUIRE( numPtr.operator==(num_ptr) );
	REQUIRE( numPtr.operator>=(num_ptr) );
	REQUIRE( numPtr.operator<=(num_ptr) );
	REQUIRE( !(numPtr.operator!=(numPtr2)) );
	REQUIRE( numPtr.operator>=(numPtr2) );
	REQUIRE( numPtr.operator<=(numPtr2) );
}

TEST_CASE("Another Test Ptr", "[base]")
{
  // Test default constructor.
  emp::Ptr<int> ptr1;
  ptr1.New();
  *ptr1 = 5;
  REQUIRE(*ptr1 == 5);
  ptr1.Delete();

  // Test pointer constructor
  int * temp_int = new int;
  emp::Ptr<int> ptr2(temp_int, true);
  *ptr2 = 10;
  REQUIRE(*ptr2 == 10);
  ptr2.Delete();

  // Test non-pointer object constructor
  int base_val = 15;
  emp::Ptr<int> ptr3(&base_val);
  REQUIRE(*ptr3 == 15);
  base_val = 20;                 // Make sure pointed to value changes with original variable.
  REQUIRE(*ptr3 == 20);

  // Test copy-constructor.
  emp::Ptr<int> ptr4(ptr3);
  REQUIRE(*ptr4 == 20);
  *ptr4 = 25;                    // Change this new pointer...
  REQUIRE(*ptr4 == 25);       // ...make sure it actually changed.
  REQUIRE(*ptr3 == 25);       // ...make sure the other pointer reflects the change.
  REQUIRE(base_val == 25);    // ...make sure the original variable changed.

  // -- Test count tracking on emp::Ptr --
  // A bit of an odd set of test... we need to create and destory pointers to make sure
  // that all of the counts are correct, so we're going to use arrays of pointers to them.

  emp::vector<emp::Ptr<char> *> ptr_set(10);
  ptr_set[0] = new emp::Ptr<char>;
  ptr_set[0]->New(42);
  for (size_t i = 1; i < 10; i++) ptr_set[i] = new emp::Ptr<char>(*(ptr_set[0]));

  // Do we have a proper count of 10?
  #ifdef EMP_TRACK_MEM
  REQUIRE(ptr_set[0]->DebugGetCount() == 10);
  ptr_set[1]->New(91);
  REQUIRE(ptr_set[0]->DebugGetCount() == 9);
  *(ptr_set[2]) = *(ptr_set[1]);
  REQUIRE(ptr_set[0]->DebugGetCount() == 8);
  REQUIRE(ptr_set[1]->DebugGetCount() == 2);

  ptr_set[3]->Delete();
  ptr_set[1]->Delete();
  #endif

  // Make sure that we are properly handling temporary pointers moved to uninitialized pointes.
  // (Previously breaking, now fixed.)
  int a = 9;
  emp::Ptr<int> ptr_a;
  ptr_a = emp::ToPtr(&a);
  int a_val = *(ptr_a);
  REQUIRE(a_val == 9);

  // Test casting to unsigned char
  emp::Ptr<uint32_t> ptr5;
  ptr5.New();
  *ptr5 = 1+1024;

  REQUIRE(*ptr5.Cast<unsigned char>() == 1);
  ptr5.Delete();

  // Test casting to const unsigned char
  emp::Ptr<uint32_t> ptr6;
  ptr6.New();
  *ptr6 = 6+1024;

  REQUIRE(*ptr6.Cast<const unsigned char>() == 6);
  ptr6.Delete();

  // Test casting to const unsigned char
  emp::Ptr<uint32_t> ptr7;
  ptr7.New();
  *ptr7 = 6+1024;
  const emp::Ptr<const unsigned char> ptr8 = ptr7.Cast<const unsigned char>();
  REQUIRE(*ptr8 == 6);
  ptr7.Delete();

  // std::cout << ptr_set[0]->DebugGetCount() << std::endl;

//   // @CAO Make sure we don't delete below 0
//   // @CAO Make sure we don't delete below 1 if we own it
//   // @CAO Make sure we only delete if you own it
//   // @CAO Make sure not to delete twice!
//   // @CAO Make sure we don't add (as owner) a pointer we already own
//
//   // -- Do some direct tests on pointer trackers --
//
//   int * real_ptr1 = new int(1);  // Count of 2 in tracker
//   int * real_ptr2 = new int(2);  // Deleted in tracker
//   int * real_ptr3 = new int(3);  // Unknown to tracker
//   int * real_ptr4 = new int(4);  // Passively known to tracker (marked non-owner)
//   auto & tracker = emp::PtrTracker::Get();
//
//   tracker.New(real_ptr1);
//   tracker.Inc(real_ptr1);
//   tracker.Inc(real_ptr1);
//   tracker.Dec(real_ptr1);
//
//   tracker.New(real_ptr2);
//   tracker.MarkDeleted(real_ptr2);
//
//   tracker.Old(real_ptr4);
//
//   REQUIRE(tracker.HasPtr(real_ptr1) == true);
//   REQUIRE(tracker.HasPtr(real_ptr2) == true);
// //  REQUIRE(tracker.HasPtr(real_ptr3) == false);  // Technically may be previous pointer re-used!
//   REQUIRE(tracker.HasPtr(real_ptr4) == true);
//
//   REQUIRE(tracker.IsActive(real_ptr1) == true);
//   REQUIRE(tracker.IsActive(real_ptr2) == false);
// //  REQUIRE(tracker.IsActive(real_ptr3) == false);
//   REQUIRE(tracker.IsActive(real_ptr4) == true);
//
//   REQUIRE(tracker.IsOwner(real_ptr1) == true);
//   REQUIRE(tracker.IsOwner(real_ptr2) == true);
// //  REQUIRE(tracker.IsOwner(real_ptr3) == false);
//   REQUIRE(tracker.IsOwner(real_ptr4) == false);
//
//   REQUIRE(tracker.GetCount(real_ptr1) == 2);
//   REQUIRE(tracker.GetCount(real_ptr2) == 1);
// //  REQUIRE(tracker.GetCount(real_ptr3) == 0);
//   REQUIRE(tracker.GetCount(real_ptr4) == 1);
}

TEST_CASE("Replicate ptr bug", "[ptr]") {
  struct testA {
    int a = 9;
    emp::Ptr<int> GetA() {return emp::ToPtr(&a);}
  };

  struct testB {
    std::function<emp::Ptr<int>(void)> b_fun;
    emp::Ptr<int> b;

    void SetBFun(std::function<emp::Ptr<int>(void)> fun) {
      b_fun = fun;
    }

    void RunBFun() {
      b = b_fun();
    }

  };

  testA ta;
  testB tb;

  std::function<emp::Ptr<int>(void)> return_a = [&ta](){return ta.GetA();};
  tb.SetBFun(return_a);
  tb.RunBFun();
  REQUIRE(*(tb.b) == 9);

}


TEST_CASE("Tests for Ptr<void> specialization.", "[base]")
{
  emp::Ptr<void> test_vptr = nullptr;
  emp::Ptr<std::string> test_sptr = emp::NewPtr<std::string>("This is my string");

  test_vptr = test_sptr.ReinterpretCast<void>();

  REQUIRE(*(test_vptr.ReinterpretCast<std::string>()) == "This is my string");

  const int x = 42;
  emp::Ptr<const int> x_ptr = &x;
  REQUIRE(*x_ptr == 42);

  emp::Ptr<const void> xv_ptr = x_ptr.ReinterpretCast<const void>();

  test_sptr.Delete();
}


//#endif // EMP_TRACK_MEM
