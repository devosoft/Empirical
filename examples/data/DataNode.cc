//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataNode demonstrating how to track different types of data.

#include <iostream>

#include "data/DataNode.h"

int main()
{
  emp::DataNode<int, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log> data;
  emp::DataNode<int, emp::data::Archive, emp::data::Current, emp::data::FullRange, emp::data::Info> data2;
  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Stats, emp::data::Log> data3;

  emp::DataNode<double> data_empty; // Build an empty DataNode to make sure no modules are required.
  data.Add(27, 28, 29);

  std::cout << "Requisites!!! ";
  emp::DataModuleRequisiteAdd<emp::data::Current, emp::data::Archive, emp::data::Stats, emp::data::FullRange>::type::PrintVals();
  std::cout << std::endl;

  std::cout << "=> Added 27, 28, and 29" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.Add(32);
  std::cout << "\n=> Added 32" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;


  data.Reset();
  std::cout << "\n=> Reset!" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.Add(100,200,300,400,500);
  std::cout << "\nAdded 100,200,300,400,500" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.AddPull([](){return -800;});
  data.PullData();
  std::cout << "\nAdded -800 via PullData()" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  data.AddPullSet([](){return emp::vector<int>({1600,0,0});});
  data.PullData();
  std::cout << "\nAdded a 1600 and two 0's via PullData()" << std::endl;
  std::cout << "Current = " << data.GetCurrent() << std::endl;
  std::cout << "Total   = " << data.GetTotal() << std::endl;
  std::cout << "Mean    = " << data.GetMean() << std::endl;
  std::cout << "Min     = " << data.GetMin() << std::endl;
  std::cout << "Max     = " << data.GetMax() << std::endl;

  std::cout << std::endl;
  data.PrintDebug();

  std::cout << std::endl;
  data2.PrintDebug();

  data2.Add(1,2,3,4,5,6,7,8,9);
  std::cout << "\nSetup data2 with values 1 through 9" << std::endl;
  std::cout << "Current = " << data2.GetCurrent() << std::endl;
  std::cout << "Total   = " << data2.GetTotal() << std::endl;
  std::cout << "Mean    = " << data2.GetMean() << std::endl;
  std::cout << "Min     = " << data2.GetMin() << std::endl;
  std::cout << "Max     = " << data2.GetMax() << std::endl;

  data2.SetInfo("Best Data", "This is the best of all possible data.", "best_data");

  std::cout << "\nSetup data3 with values 1 through 9" << std::endl;

  data3.Add(1,2,3,4,5,6,7,8,8);
  std::cout << "Current = " << data3.GetCurrent() << std::endl;
  std::cout << "Total   = " << data3.GetTotal() << std::endl;
  std::cout << "Mean    = " << data3.GetMean() << std::endl;
  std::cout << "Min     = " << data3.GetMin() << std::endl;
  std::cout << "Max     = " << data3.GetMax() << std::endl;
  std::cout << "Variance= " << data3.GetVariance() << std::endl;
  std::cout << "Std Dev = " << data3.GetStandardDeviation() << std::endl;
  std::cout << "Skew    = " << data3.GetSkew() << std::endl;
  std::cout << "Kurtosis= " << data3.GetKurtosis() << std::endl;


  emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Histogram, emp::data::Pull, emp::data::Log> hist_data;
  hist_data.SetupBins(1,21,10);
  hist_data.Add(1,2,1,19);
  
  std::cout << "hist_data.GetHistMin() = " << hist_data.GetHistMin()
	    << "; expected 1" << std::endl;
  std::cout << "hist_data.GetHistWidth(5) = " << hist_data.GetHistWidth(5)
	    << "; expected 2" << std::endl;
  
  std::cout << "hist_data.GetBinMins() = " << emp::to_string(hist_data.GetBinMins())
	    << "; expected emp::vector<double>({1,3,5,7,9,11,13,15,17,19}" << std::endl;
  
  std::cout << "hist_data.GetHistCount(9) = " << hist_data.GetHistCount(9)
	    << "; expected 1" << std::endl;
  std::cout << "hist_data.GetHistCounts() = " << emp::to_string(hist_data.GetHistCounts())
	    << "; expected emp::vector<size_t>({3,0,0,0,0,0,0,0,0,1}" << std::endl;
  
  hist_data.Reset();
  std::cout << "hist_data.GetHistCounts() = " << emp::to_string(hist_data.GetHistCounts())
	    << "; expected emp::vector<size_t>({0,0,0,0,0,0,0,0,0,0}" << std::endl;
  
}
