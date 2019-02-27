//#ifndef EMP_TRACK_MEM
//#define EMP_TRACK_MEM

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "base/Ptr.h"

#include <sstream>
#include <iostream>

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
	tracker.NewArray(arr1, 5);
	REQUIRE(tracker.HasPtr(arr1));
	size_t arr1ID = tracker.GetCurID(arr1);
	REQUIRE(tracker.IsActiveID(arr1ID));
	
	emp::Ptr<int> arr1ptr(arr1, 5, false);
	REQUIRE(!arr1ptr.IsNull());
	
	// attempts to delete const array fails, error: "discards qualifiers"
	
	arr1ptr.NewArray(10);
	//REQUIRE(arr1ptr.DebugGetArrayBytes() == (10 * sizeof(int)));
	
	arr1ptr.DeleteArray();
	
	tracker.MarkDeleted(arr1ID);
	REQUIRE(!tracker.IsActiveID(arr1ID));
	size_t arr1ptrID = tracker.GetCurID(arr1ptr);
	tracker.MarkDeleted(arr1ptrID);
	tracker.MarkDeleted(tracker.GetCurID(arr));
	
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

//#endif // EMP_TRACK_MEM