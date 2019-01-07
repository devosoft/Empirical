#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/vector_utils.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test vector_utils", "[tools]")
{
	// RemoveValue
	emp::vector<int> v_int;
	v_int.push_back(1);
	v_int.push_back(3);
	v_int.push_back(5);
	v_int.push_back(9);
	REQUIRE(emp::RemoveValue(v_int, 9));
	REQUIRE(v_int.size() == 3);
	
	// Has
	REQUIRE(!emp::Has(v_int, 9));
	REQUIRE(emp::Has(v_int, 1));
	REQUIRE(emp::Has(v_int, 3));
	REQUIRE(emp::Has(v_int, 5));
	
	// Print
	std::stringstream ss;
	emp::Print(v_int, ss);
	REQUIRE(ss.str() == "1 3 5");
	
	// FindMinIndex FindMaxIndex
	REQUIRE(emp::FindMinIndex(v_int) == 0);
	REQUIRE(emp::FindMaxIndex(v_int) == 2);
	v_int.push_back(11);
	v_int.push_back(0);
	REQUIRE(emp::FindMaxIndex(v_int) == 3);
	REQUIRE(emp::FindMinIndex(v_int) == 4);
	
	// Sum Product
	REQUIRE(emp::Sum(v_int) == 20);
	REQUIRE(emp::Product(v_int) == 0);
	emp::RemoveValue(v_int, 0);
	REQUIRE(emp::Product(v_int) == 165);
	
	// Sort
	emp::vector<double> v_double;
	v_double.push_back(10);
	v_double.push_back(5);
	v_double.push_back(50);
	v_double.push_back(20);
	v_double.push_back(45);
	REQUIRE(v_double.at(0) == 10);
	emp::Sort(v_double);
	REQUIRE(v_double.at(0) == 5);
	
	// Slice
	emp::vector<double> v_d2 = emp::Slice(v_double, 0, 3);
	REQUIRE(v_d2.size() == 3);
	REQUIRE(emp::Has(v_d2, 5.0));
	REQUIRE(emp::Has(v_d2, 10.0));
	REQUIRE(emp::Has(v_d2, 20.0));
	
	// Heapify (max heap)
	emp::Heapify(v_d2);
	REQUIRE(v_d2.at(0) == 20.0);
	REQUIRE(v_d2.at(1) == 10.0);
	REQUIRE(v_d2.at(2) == 5.0);
  
  // Heapify should change nothing
  emp::vector<double> v_d3 = emp::Slice(v_d2, 0, 2);
  emp::Heapify(v_d3);
  REQUIRE(v_d2.at(0) == 20.0);
	REQUIRE(v_d2.at(1) == 10.0);
	
	// HeapExtract
	REQUIRE(emp::HeapExtract(v_d3) == 20.0);
	REQUIRE(emp::HeapExtract(v_d3) == 10.0);
	REQUIRE(v_d3.size() == 0);
	
	// HeapInsert
	emp::HeapInsert(v_d2, 35.0);
	REQUIRE(v_d2.at(0) == 35.0);
	
	
	
	
	
	
	
}