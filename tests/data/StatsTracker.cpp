/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "emp/data/StatsTracker.hpp"

TEST_CASE("Test StatsTracker Concepts", "[data]")
{
  REQUIRE(emp::Numeric<int>);
  REQUIRE(emp::Numeric<double>);
  REQUIRE(!emp::Numeric<std::string>);
}

TEST_CASE("Test Empty StatsTracker", "[data]")
{
  emp::StatsTracker<double> stats;

  REQUIRE(!stats.HasData());
  REQUIRE(stats.GetCount() == 0);
  REQUIRE(stats.GetTotal() == 0.0);
  REQUIRE(stats.GetMin() == 0.0);
  REQUIRE(stats.GetMax() == 0.0);
  REQUIRE(stats.GetLast() == 0.0);
  REQUIRE(stats.GetAverage() == 0.0);
}

TEST_CASE("Test StatsTracker Accumulates Values", "[data]")
{
  emp::StatsTracker<int> stats;

  stats.Add(4);
  stats.Add(-2);
  stats.Add(10);

  REQUIRE(stats.HasData());
  REQUIRE(stats.GetCount() == 3);
  REQUIRE(stats.GetTotal() == 12);
  REQUIRE(stats.GetMin() == -2);
  REQUIRE(stats.GetMax() == 10);
  REQUIRE(stats.GetLast() == 10);
  REQUIRE(stats.GetAverage() == 4.0);
}

TEST_CASE("Test StatsTracker Reset", "[data]")
{
  emp::StatsTracker<double> stats;

  stats.Add(1.5);
  stats.Add(2.5);
  stats.Reset();

  REQUIRE(!stats.HasData());
  REQUIRE(stats.GetCount() == 0);
  REQUIRE(stats.GetTotal() == 0.0);
  REQUIRE(stats.GetMin() == 0.0);
  REQUIRE(stats.GetMax() == 0.0);
  REQUIRE(stats.GetLast() == 0.0);
  REQUIRE(stats.GetAverage() == 0.0);
}

TEST_CASE("Test StatsTracker Printing", "[data]")
{
  emp::StatsTracker<double> stats;
  stats.Add(2.0);
  stats.Add(4.0);

  std::ostringstream os;
  stats.Print(os, "Example");

  REQUIRE(os.str().find("Example") != std::string::npos);
  REQUIRE(os.str().find("2 samples") != std::string::npos);
  REQUIRE(os.str().find("total 6") != std::string::npos);
}
