/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file DataInterface.cpp
 */

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/DataFile.hpp"
#include "emp/data/DataInterface.hpp"
#include "emp/data/DataManager.hpp"
#include "emp/data/DataNode.hpp"

TEST_CASE("Test DataInterface", "[data]") {
  auto * di = emp::MakeDataInterface<double, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log>();
  delete di;

  emp::DataNode<double, emp::data::Current, emp::data::Range> node;
  node.Add(5.5, .6); // Put in some test data, since we can't add through the interface

  auto * di2(&node);
  REQUIRE(di2->GetTotal() == 6.1);
  REQUIRE(di2->GetMin() == .6);
  REQUIRE(di2->GetMax() == 5.5);
  REQUIRE(di2->GetMean() == 3.05);

  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Stats> node2;
  node2.Add(5.5, .6); // Put in some test data, since we can't add through the interface

  auto * di3(&node2);

  REQUIRE(di3->GetTotal() == 6.1);
  REQUIRE(di3->GetMin() == .6);
  REQUIRE(di3->GetMax() == 5.5);
  REQUIRE(di3->GetMean() == 3.05);
  REQUIRE(di3->GetVariance() == Approx(6.0025));
  REQUIRE(di3->GetStandardDeviation() == Approx(2.45));
  REQUIRE(di3->GetSkew() == 0);
  REQUIRE(di3->GetKurtosis() == -2);
}
