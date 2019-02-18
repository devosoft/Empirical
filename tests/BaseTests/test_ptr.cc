#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM

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
	
	int arr2[5] = {1, 2, 3, 4, 5};
	const emp::Ptr<int> arr2constptr(arr2, 5, true);
	REQUIRE( (*arr2constptr) == 1 );
	
	REQUIRE( arr2constptr != arr1ptr );
	REQUIRE( arr2constptr >= arr2constptr);
	REQUIRE( arr2constptr <= arr2constptr);
	REQUIRE( arr2constptr >= arr2 );
	REQUIRE( arr2constptr <= arr2 );
	
	// can't delete array because it discards qualifiers
	//arr2constptr.DeleteArray();
	
	arr1ptr.NewArray(10);
	REQUIRE(arr1ptr.DebugGetArrayBytes() == (10 * sizeof(int)));
	
	arr1ptr.DeleteArray();
	
	tracker.MarkDeleted(arr1ID);
	REQUIRE(!tracker.IsActiveID(arr1ID));
}

#endif // EMP_TRACK_MEM