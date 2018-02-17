#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#undef NDEBUG
#define TDEBUG 1

#include "third-party/Catch/single_include/catch.hpp"

#include "data/DataNode.h"
#include <iostream>

TEST_CASE("Test DataRange", "[data]") {
    emp::DataNode<int, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log> data;

    data.Add(27, 28, 29);

    std::cout << "=> Added 27, 28, and 29" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == 29);
    REQUIRE(data.GetTotal() == 84);
    REQUIRE(data.GetMean() == 28);
    REQUIRE(data.GetMin() == 27);
    REQUIRE(data.GetMax() == 29);

    data.Add(32);
    std::cout << "\n=> Added 32" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == 32);
    REQUIRE(data.GetTotal() == 116);
    REQUIRE(data.GetMean() == 29);
    REQUIRE(data.GetMin() == 27);
    REQUIRE(data.GetMax() == 32);


    data.Reset();
    std::cout << "\n=> Reset!" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == 32);
    REQUIRE(data.GetTotal() == 0);
    REQUIRE(isnan(data.GetMean()));
    REQUIRE(data.GetMin() == 0);
    REQUIRE(data.GetMax() == 0);

    data.Add(100,200,300,400,500);
    std::cout << "\nAdded 100,200,300,400,500" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == 500);
    REQUIRE(data.GetTotal() == 1500);
    REQUIRE(data.GetMean() == 300);
    REQUIRE(data.GetMin() == 100);
    REQUIRE(data.GetMax() == 500);

    data.AddPull([](){return -800;});
    data.PullData();
    std::cout << "\nAdded -800 via PullData()" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == -800);
    REQUIRE(data.GetTotal() == 700);
    REQUIRE(data.GetMean() == Approx(116.6667));
    REQUIRE(data.GetMin() == -800);
    REQUIRE(data.GetMax() == 500);

    data.AddPullSet([](){return emp::vector<int>({1600,0,0});});
    data.PullData(); // Remember that this also runs the function that returns -800
    std::cout << "\nAdded another -800, a 1600 and two 0's via PullData()" << std::endl;
    std::cout << "Current = " << data.GetCurrent() << std::endl;
    std::cout << "Total   = " << data.GetTotal() << std::endl;
    std::cout << "Mean    = " << data.GetMean() << std::endl;
    std::cout << "Min     = " << data.GetMin() << std::endl;
    std::cout << "Max     = " << data.GetMax() << std::endl;

    REQUIRE(data.GetCurrent() == 0);
    REQUIRE(data.GetTotal() == 1500);
    REQUIRE(data.GetMean() == 150);
    REQUIRE(data.GetMin() == -800);
    REQUIRE(data.GetMax() == 1600);

    std::cout << std::endl;
    data.PrintDebug();

    std::cout << std::endl;

}

TEST_CASE("Test DataArchive", "[data]") {
    emp::DataNode<int, emp::data::Archive, emp::data::Current, emp::data::FullRange, emp::data::Info> data2;

    data2.PrintDebug();

    data2.Add(1,2,3,4,5,6,7,9,8);
    std::cout << "\nSetup data2 with values 1 through 9" << std::endl;
    std::cout << "Current = " << data2.GetCurrent() << std::endl;
    std::cout << "Total   = " << data2.GetTotal() << std::endl;
    std::cout << "Mean    = " << data2.GetMean() << std::endl;
    std::cout << "Min     = " << data2.GetMin() << std::endl;
    std::cout << "Max     = " << data2.GetMax() << std::endl;

    REQUIRE(data2.GetCurrent() == 8);
    REQUIRE(data2.GetTotal() == 45);
    REQUIRE(data2.GetMean() == 5);
    REQUIRE(data2.GetMin() == 1);
    REQUIRE(data2.GetMax() == 9);

    data2.SetInfo("Best Data", "This is the best of all possible data.", "best_data");
    REQUIRE(data2.GetName() == "Best Data");
    REQUIRE(data2.GetDescription() == "This is the best of all possible data.");
    REQUIRE(data2.GetKeyword() == "best_data");
}

TEST_CASE("Test DataStats", "[data]") {
    emp::DataNode<double, emp::data::Current, emp::data::Range, emp::data::Stats, emp::data::Log> data3;

    std::cout << "\nSetup data3 with values 1 through 8 with an extra 8" << std::endl;

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

    REQUIRE(data3.GetCurrent() == 8);
    REQUIRE(data3.GetTotal() == 44);
    REQUIRE(data3.GetMean() == Approx(4.8888888888889));
    REQUIRE(data3.GetMin() == 1);
    REQUIRE(data3.GetMax() == 8);
    REQUIRE(data3.GetVariance() == Approx(5.87654));
    REQUIRE(data3.GetStandardDeviation() == Approx(2.42416));
    REQUIRE(data3.GetSkew() == Approx(-0.151045));
    REQUIRE(data3.GetKurtosis() == Approx(-1.3253830944));
}

TEST_CASE("Test DataEmpty", "[data]") {
    emp::DataNode<double> data_empty; // Build an empty DataNode to make sure no modules are required.
}