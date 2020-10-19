#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/DataNode.hpp"
#include "emp/data/DataManager.hpp"
#include "emp/data/DataInterface.hpp"
#include "emp/data/DataFile.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

TEST_CASE("Test DataManager", "[data]") {
    emp::DataManager<double, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log> dataM;

    dataM.New("fitness");
    dataM.New("merit");
    dataM.New("fidelity");

    dataM.AddData("fitness", 3.0);
    dataM.Get("fitness").Add(6.5);
    auto & fit_node = dataM.Get("fitness");
    fit_node.Add(10.0);

    auto & merit_node = dataM.Get("merit");
    merit_node.Add(300, 650, 500);

    dataM.AddData("fidelity", 1.0, 0.8, 0.9);

    // std::cout << std::endl;

    // for (auto & x : dataM.GetNodes()) {
    //     auto & node = *(x.second);
    //     std::cout << x.first
    //             << " cur=" << node.GetCurrent()
    //             << " ave=" << node.GetMean()
    //             << " min=" << node.GetMin()
    //             << " max=" << node.GetMax()
    //             << " tot=" << node.GetTotal()
    //             << std::endl;
    // }

    REQUIRE(dataM.Get("fitness").GetCurrent() == 10);
    REQUIRE(dataM.Get("fitness").GetMean() == 6.5);
    REQUIRE(dataM.Get("fitness").GetMin() == 3);
    REQUIRE(dataM.Get("fitness").GetMax() == 10);
    REQUIRE(dataM.Get("fitness").GetTotal() == 19.5);

    REQUIRE(dataM.Get("fidelity").GetCurrent() == .9);
    REQUIRE(dataM.Get("fidelity").GetMean() == .9);
    REQUIRE(dataM.Get("fidelity").GetMin() == .8);
    REQUIRE(dataM.Get("fidelity").GetMax() == 1);
    REQUIRE(dataM.Get("fidelity").GetTotal() == 2.7);

}