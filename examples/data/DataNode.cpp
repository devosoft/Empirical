//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataNode demonstrating how to track different types of data.

#include <iostream>

#include "emp/data/DataNode.hpp"

int main()
{
  emp::DataNode<int, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log> data;
  emp::DataNode<int, emp::data::Archive, emp::data::Current, emp::data::FullRange, emp::data::Info> data2;
  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Stats, emp::data::Log> data3;

  emp::DataNode<double> data_empty; // Build an empty DataNode to make sure no modules are required.
  data.Add(27, 28, 29);

  std::cout << "Requisites!!! ";
  emp::DataModuleRequisiteAdd<emp::data::Current, emp::data::Archive, emp::data::Stats, emp::data::FullRange>::type::PrintVals();
  std::cout << '\n';

  std::cout << "=> Added 27, 28, and 29" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';

  data.Add(32);
  std::cout << "\n=> Added 32" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';


  data.Reset();
  std::cout << "\n=> Reset!" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';

  data.Add(100,200,300,400,500);
  std::cout << "\nAdded 100,200,300,400,500" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';

  data.AddPull([](){return -800;});
  data.PullData();
  std::cout << "\nAdded -800 via PullData()" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';

  data.AddPullSet([](){return emp::vector<int>({1600,0,0});});
  data.PullData();
  std::cout << "\nAdded a 1600 and two 0's via PullData()" << '\n';
  std::cout << "Current = " << data.GetCurrent() << '\n';
  std::cout << "Total   = " << data.GetTotal() << '\n';
  std::cout << "Mean    = " << data.GetMean() << '\n';
  std::cout << "Min     = " << data.GetMin() << '\n';
  std::cout << "Max     = " << data.GetMax() << '\n';

  std::cout << '\n';
  data.PrintDebug();

  std::cout << '\n';
  data2.PrintDebug();

  data2.Add(1,2,3,4,5,6,7,8,9);
  std::cout << "\nSetup data2 with values 1 through 9" << '\n';
  std::cout << "Current = " << data2.GetCurrent() << '\n';
  std::cout << "Total   = " << data2.GetTotal() << '\n';
  std::cout << "Mean    = " << data2.GetMean() << '\n';
  std::cout << "Min     = " << data2.GetMin() << '\n';
  std::cout << "Max     = " << data2.GetMax() << '\n';

  data2.SetInfo("Best Data", "This is the best of all possible data.", "best_data");

  std::cout << "\nSetup data3 with values 1 through 9" << '\n';

  data3.Add(1,2,3,4,5,6,7,8,8);
  std::cout << "Current = " << data3.GetCurrent() << '\n';
  std::cout << "Total   = " << data3.GetTotal() << '\n';
  std::cout << "Mean    = " << data3.GetMean() << '\n';
  std::cout << "Min     = " << data3.GetMin() << '\n';
  std::cout << "Max     = " << data3.GetMax() << '\n';
  std::cout << "Variance= " << data3.GetVariance() << '\n';
  std::cout << "Std Dev = " << data3.GetStandardDeviation() << '\n';
  std::cout << "Skew    = " << data3.GetSkew() << '\n';
  std::cout << "Kurtosis= " << data3.GetKurtosis() << '\n';


  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Histogram, emp::data::Pull, emp::data::Log> hist_data;
  hist_data.SetupBins(1,21,10);
  hist_data.Add(1,2,1,19);

  std::cout << "hist_data.GetHistMin() = " << hist_data.GetHistMin()
	    << "; expected 1" << '\n';
  std::cout << "hist_data.GetHistWidth(5) = " << hist_data.GetHistWidth(5)
	    << "; expected 2" << '\n';

  std::cout << "hist_data.GetBinMins() = " << emp::to_string(hist_data.GetBinMins())
	    << "; expected emp::vector<double>({1,3,5,7,9,11,13,15,17,19}" << '\n';

  std::cout << "hist_data.GetHistCount(9) = " << hist_data.GetHistCount(9)
	    << "; expected 1" << '\n';
  std::cout << "hist_data.GetHistCounts() = " << emp::to_string(hist_data.GetHistCounts())
	    << "; expected emp::vector<size_t>({3,0,0,0,0,0,0,0,0,1}" << '\n';

  hist_data.Reset();
  std::cout << "hist_data.GetHistCounts() = " << emp::to_string(hist_data.GetHistCounts())
	    << "; expected emp::vector<size_t>({0,0,0,0,0,0,0,0,0,0}" << '\n';

}
