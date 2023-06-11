/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file vector_utils.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/vector_utils.hpp"

TEST_CASE("Test vector_utils", "[datastructs]")
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

  emp::vector<int> range_vec = emp::NRange(4, 7);
  REQUIRE(range_vec[0] == 4);
  REQUIRE(range_vec[1] == 5);
  REQUIRE(range_vec[2] == 6);
  REQUIRE(range_vec.size() == 3);

  // RemoveDuplicates
  range_vec.push_back(4);
  REQUIRE(range_vec.size() == 4);
  range_vec = emp::RemoveDuplicates(range_vec);
  REQUIRE(range_vec.size() == 3);

  // Flatten
  emp::vector<emp::vector<int>> nested_v = {{2,1,6}, {4,5,3}};
  emp::vector<int> flattened_v = emp::Flatten(nested_v);
  REQUIRE(flattened_v[0] == 2);
  REQUIRE(flattened_v[1] == 1);
  REQUIRE(flattened_v[2] == 6);
  REQUIRE(flattened_v[3] == 4);
  REQUIRE(flattened_v[4] == 5);
  REQUIRE(flattened_v[5] == 3);

  // FindMin and FindMax
  REQUIRE(emp::FindMax(flattened_v) == 6);
  REQUIRE(emp::FindMin(flattened_v) == 1);

  // Concat
  nested_v = emp::Concat(nested_v, range_vec);
  REQUIRE(nested_v[0][0] == 2);
  REQUIRE(nested_v[0][1] == 1);
  REQUIRE(nested_v[0][2] == 6);
  REQUIRE(nested_v[1][0] == 4);
  REQUIRE(nested_v[1][1] == 5);
  REQUIRE(nested_v[1][2] == 3);
  REQUIRE(nested_v[2][0] == 4);
  REQUIRE(nested_v[2][1] == 5);
  REQUIRE(nested_v[2][2] == 6);

  // FindEval
  std::function<bool(int)> is_even = [](int i){return ((i % 2) == 0);};
  REQUIRE(emp::FindEval(flattened_v, is_even, 1) == 2);

  // Scale
  emp::Scale(range_vec, 2);
  REQUIRE(range_vec[0] == 8);
  REQUIRE(range_vec[1] == 10);
  REQUIRE(range_vec[2] == 12);

  // Heapify on a larger vector
  emp::Heapify(flattened_v);
  REQUIRE(flattened_v.at(0) == 6);
  REQUIRE(flattened_v.at(0) > flattened_v.at(1));
  REQUIRE(flattened_v.at(0) > flattened_v.at(2));
  REQUIRE(flattened_v.at(1) > flattened_v.at(3));
  REQUIRE(flattened_v.at(1) > flattened_v.at(4));
  REQUIRE(flattened_v.at(2) > flattened_v.at(5));
}


TEST_CASE("Another Test vector utils", "[datastructs]") {
  emp::vector<int> v1({6,2,5,1,3});
  emp::vector<int> v2({7,6,7,1,7});
  emp::Sort(v1);
  REQUIRE(v1 == emp::vector<int>({1,2,3,5,6}));
  REQUIRE(emp::FindValue(v1, 3) == 2);
  REQUIRE(emp::Sum(v1) == 17);
  REQUIRE(emp::Has(v1, 3));
  REQUIRE(!emp::Has(v1, 4));
  REQUIRE(emp::Product(v1) == 180);
  REQUIRE(emp::Slice(v1,1,3) == emp::vector<int>({2,3}));
  REQUIRE(emp::Count(v1, 2) == 1);
  REQUIRE(emp::Count(v2, 7) == 3);

  // Test handling vector-of-vectors.
  using vv_int_t = emp::vector< emp::vector< int > >;
  vv_int_t vv = {{1,2,3},{4,5,6},{7,8,9}};
  vv_int_t vv2 = emp::Transpose(vv);
  REQUIRE(vv[0][2] == 3);
  REQUIRE(vv[1][0] == 4);
  REQUIRE(vv2[0][2] == 7);
  REQUIRE(vv2[1][0] == 2);

  // Test build range
  emp::vector<size_t> vr = emp::BuildRange<size_t>(1, 5);
  REQUIRE(vr.size() == 5);
  REQUIRE(vr[0] == 1);

}
