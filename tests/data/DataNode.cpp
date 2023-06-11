/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file DataNode.cpp
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

TEST_CASE("Test DataNode", "[data]") {
  // Create a new empty DataNode
  emp::DataNode<int, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log, emp::data::Info> data;
  // Test GetCount function before and after adding to the node
  REQUIRE(data.GetCount()==0);
  data.Add(27, 28, 29);
  REQUIRE(data.GetCount()==3);
  data.Reset();
  REQUIRE(data.GetCount()==0);
  // ResetCount is untracked so should always be 0
  REQUIRE(data.GetResetCount()==0);

  const std::string info = "test data node";
  data.SetDescription(info);
  data.SetName(info);
  data.SetKeyword(info);
  REQUIRE(data.GetDescription()==info);
  REQUIRE(data.GetName()==info);
  REQUIRE(data.GetKeyword()==info);
}

TEST_CASE("Test DataRange", "[data]") {
  emp::DataNode<int, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log> data;

  data.Add(27);
  REQUIRE(data.GetCurrent() == 27);
  REQUIRE(data.GetTotal() == 27);
  REQUIRE(data.GetMean() == 27);
  REQUIRE(data.GetMin() == 27);
  REQUIRE(data.GetMax() == 27);
  REQUIRE(data.GetMedian() == 27);
  REQUIRE(data.GetPercentile(0) == 27);
  REQUIRE(data.GetPercentile(25) == 27);
  REQUIRE(data.GetPercentile(100) == 27);

  data.Add(29,28);

  // std::cout << "=> Added 27, 28, and 29" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == 28);
  REQUIRE(data.GetTotal() == 84);
  REQUIRE(data.GetMean() == 28);
  REQUIRE(data.GetMin() == 27);
  REQUIRE(data.GetMax() == 29);
  REQUIRE(data.GetMedian() == 28);
  REQUIRE(data.GetPercentile(0) == 27);
  REQUIRE(data.GetPercentile(25) == 27.5);
  REQUIRE(data.GetPercentile(100) == 29);

  data.Add(32);
  // std::cout << "\n=> Added 32" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == 32);
  REQUIRE(data.GetTotal() == 116);
  REQUIRE(data.GetMean() == 29);
  REQUIRE(data.GetMin() == 27);
  REQUIRE(data.GetMax() == 32);
  REQUIRE(data.GetMedian() == 28.5);
  REQUIRE(data.GetPercentile(0) == 27);
  REQUIRE(data.GetPercentile(25) == 27.75);
  REQUIRE(data.GetPercentile(100) == 32);


  data.Reset();
  // std::cout << "\n=> Reset!" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == 32);
  REQUIRE(data.GetTotal() == 0);
  REQUIRE(std::isnan(data.GetMean()));
  REQUIRE(data.GetMin() == 0);
  REQUIRE(data.GetMax() == 0);
  REQUIRE(std::isnan(data.GetMedian()));
  REQUIRE(std::isnan(data.GetPercentile(0)));
  REQUIRE(std::isnan(data.GetPercentile(25)));
  REQUIRE(std::isnan(data.GetPercentile(100)));

  data.Add(100,200,300,400,500);
  // std::cout << "\nAdded 100,200,300,400,500" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == 500);
  REQUIRE(data.GetTotal() == 1500);
  REQUIRE(data.GetMean() == 300);
  REQUIRE(data.GetMin() == 100);
  REQUIRE(data.GetMax() == 500);
  REQUIRE(data.GetMedian() == 300);
  REQUIRE(data.GetPercentile(0) == 100);
  REQUIRE(data.GetPercentile(25) == 200);
  REQUIRE(data.GetPercentile(100) == 500);

  data.AddPull([](){return -800;});
  data.PullData();
  // std::cout << "\nAdded -800 via PullData()" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == -800);
  REQUIRE(data.GetTotal() == 700);
  REQUIRE(data.GetMean() == Approx(116.6667));
  REQUIRE(data.GetMin() == -800);
  REQUIRE(data.GetMax() == 500);
  REQUIRE(data.GetMedian() == 250);
  REQUIRE(data.GetPercentile(0) == -800);
  REQUIRE(data.GetPercentile(25) == 125);
  REQUIRE(data.GetPercentile(100) == 500);

  data.AddPullSet([](){return emp::vector<int>({1600,0,0});});
  data.PullData(); // Remember that this also runs the function that returns -800
  // std::cout << "\nAdded another -800, a 1600 and two 0's via PullData()" << std::endl;
  // std::cout << "Current = " << data.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data.GetMean() << std::endl;
  // std::cout << "Min   = " << data.GetMin() << std::endl;
  // std::cout << "Max   = " << data.GetMax() << std::endl;

  REQUIRE(data.GetCurrent() == 0);
  REQUIRE(data.GetTotal() == 1500);
  REQUIRE(data.GetMean() == 150);
  REQUIRE(data.GetMin() == -800);
  REQUIRE(data.GetMax() == 1600);
  REQUIRE(data.GetMedian() == 150);
  REQUIRE(data.GetPercentile(0) == -800);
  REQUIRE(data.GetPercentile(25) == 0);
  REQUIRE(data.GetPercentile(100) == 1600);

  std::stringstream result;

  data.PrintDebug(result);
  REQUIRE(result.str() == "Main DataNode.\nDataNodeModule for data::Pull. (level 8)\nDataNodeModule for data::Range. (level 4)\nDataNodeModule for data::Log. (level 2)\nDataNodeModule for data::Current. (level 0)\nBASE DataNodeModule.\n");
  result.str("");

  data.PrintLog(result);
  REQUIRE(result.str() == "100, 200, 300, 400, 500, -800, -800, 1600, 0, 0\n");
  result.str("");

  data.Add(5);
  data.PrintCurrent(result);
  REQUIRE(result.str() == "5");

}

TEST_CASE("Test DataEmpty", "[data]") {
  emp::DataNode<double> data_empty; // Build an empty DataNode to make sure no modules are required.
}

TEST_CASE("Test DataArchive", "[data]") {
  emp::DataNode<int, emp::data::Archive, emp::data::Current, emp::data::FullRange, emp::data::Info> data2;

  // data2.PrintDebug();

  data2.Add(1,2,3,4,5,6,7,9,8);
  // std::cout << "\nSetup data2 with values 1 through 9" << std::endl;
  // std::cout << "Current = " << data2.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data2.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data2.GetMean() << std::endl;
  // std::cout << "Min   = " << data2.GetMin() << std::endl;
  // std::cout << "Max   = " << data2.GetMax() << std::endl;

  REQUIRE(data2.GetCurrent() == 8);
  REQUIRE(data2.GetTotal() == 45);
  REQUIRE(data2.GetMean() == 5);
  REQUIRE(data2.GetMin() == 1);
  REQUIRE(data2.GetMax() == 9);
  REQUIRE(data2.GetData() == emp::vector<int>({1,2,3,4,5,6,7,9,8}));

  data2.SetInfo("Best Data", "This is the best of all possible data.", "best_data");

  // Test reset
  data2.Reset();

  // Generate what the archive should look like now that we Reset
  emp::vector<emp::vector<int>> arch_comp;
  arch_comp.push_back(emp::vector<int>({1,2,3,4,5,6,7,9,8}));

  // Compare archives as strings for easy reading of errors.
  REQUIRE(emp::to_string(data2.GetArchive()) == emp::to_string(arch_comp));

  data2.Add(4);
  arch_comp.push_back(emp::vector<int>({4}));

  REQUIRE(data2.GetCurrent() == 4);
  REQUIRE(data2.GetTotal() == 4);
  REQUIRE(data2.GetMean() == 4);
  REQUIRE(data2.GetMin() == 4);
  REQUIRE(data2.GetMax() == 4);
  REQUIRE(data2.GetData() == emp::vector<int>({4}));
  REQUIRE(data2.GetData(0) == emp::vector<int>({1,2,3,4,5,6,7,9,8}));
  REQUIRE(data2.GetResetCount() == 1);


  data2.Reset();
  REQUIRE(data2.GetArchive() == arch_comp);

  // Test that setting info worked
  REQUIRE(data2.GetName() == "Best Data");
  REQUIRE(data2.GetDescription() == "This is the best of all possible data.");
  REQUIRE(data2.GetKeyword() == "best_data");

}

TEST_CASE("Test DataStats", "[data]") {
  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Stats, emp::data::Log> data3;

  // std::cout << "\nSetup data3 with values 1 through 8 with an extra 8" << std::endl;

  data3.Add(1,2,3,4,5,6,7,8,8);
  // std::cout << "Current = " << data3.GetCurrent() << std::endl;
  // std::cout << "Total   = " << data3.GetTotal() << std::endl;
  // std::cout << "Mean  = " << data3.GetMean() << std::endl;
  // std::cout << "Min   = " << data3.GetMin() << std::endl;
  // std::cout << "Max   = " << data3.GetMax() << std::endl;
  // std::cout << "Variance= " << data3.GetVariance() << std::endl;
  // std::cout << "Std Dev = " << data3.GetStandardDeviation() << std::endl;
  // std::cout << "Skew  = " << data3.GetSkew() << std::endl;
  // std::cout << "Kurtosis= " << data3.GetKurtosis() << std::endl;

  REQUIRE(data3.GetCurrent() == 8);
  REQUIRE(data3.GetTotal() == 44);
  REQUIRE(data3.GetMean() == Approx(4.8888888888889));
  REQUIRE(data3.GetMin() == 1);
  REQUIRE(data3.GetMax() == 8);
  REQUIRE(data3.GetVariance() == Approx(5.87654));
  REQUIRE(data3.GetStandardDeviation() == Approx(2.42416));
  // skewness calculation: https://www.wolframalpha.com/input/?i=skewness+1%2C2%2C3%2C4%2C5%2C6%2C7%2C8%2C8
  // allow fallback to less precise constexpr Pow
  const bool skew_res{ data3.GetSkew() == Approx(-0.150986) || data3.GetSkew() == Approx(-0.151045) };
  REQUIRE( skew_res );
  // kurtosis calculateion
  // - https://www.wolframalpha.com/input/?i=N%5BKurtosis%5B%7B1%2C+2%2C+3%2C+4%2C+5%2C+6%2C+7%2C+8%2C+8%7D%5D%5D+-+3
  // - https://www.itl.nist.gov/div898/handbook/eda/section3/eda35b.htm
  // allow fallback to less precise constexpr Pow
  const bool kurtosis_res{ data3.GetKurtosis() == Approx(-1.325383) || data3.GetKurtosis() == Approx(-1.3253830944) };
  REQUIRE( kurtosis_res );

}

TEST_CASE("Test histogram", "[data]") {
  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Histogram, emp::data::Pull, emp::data::Log> data;
  data.SetupBins(1,21,10);
  data.Add(1,2,1,19, 0, -1, 49);

  REQUIRE(data.GetHistMin() == 1);
  REQUIRE(data.GetHistWidth(5) == 2);
  REQUIRE(data.GetHistMax() == 21);

  REQUIRE(data.GetBinMins() == emp::vector<double>({1,3,5,7,9,11,13,15,17,19}));

  REQUIRE(data.GetOverflow() == 1);
  REQUIRE(data.GetUnderflow() == 2);
  REQUIRE(data.GetHistCount(9) == 1);
  REQUIRE(data.GetHistCounts() == emp::vector<size_t>({3,0,0,0,0,0,0,0,0,1}));

  data.Reset();
  REQUIRE(data.GetHistCounts() == emp::vector<size_t>({0,0,0,0,0,0,0,0,0,0}));

  std::stringstream result;
  data.PrintDebug(result);
  REQUIRE(result.str() == "Main DataNode.\nDataNodeModule for data::Pull. (level 8)\nDataNodeModule for data::Histogram. (level 5)\nDataNodeModule for data::Range. (level 4)\nDataNodeModule for data::Log. (level 2)\nDataNodeModule for data::Current. (level 0)\nBASE DataNodeModule.\n");
}
