/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <chrono>
#include <sstream>
#include <thread>

#include "emp/tools/Timer.hpp"

TEST_CASE("Test RAII Timer", "[tools]")
{
  using TestTimer = emp::Timer<"RAII Timer Test">;

  TestTimer::ResetStats();
  REQUIRE(TestTimer::GetName() == "RAII Timer Test");

  {
    TestTimer timer;
    REQUIRE(timer.IsRunning());
    REQUIRE(!timer.IsPaused());
  }

  REQUIRE(TestTimer::GetStats().GetCount() == 1);
  REQUIRE(TestTimer::GetStats().GetMin() >= 0.0);
  REQUIRE(TestTimer::GetStats().GetMax() >= TestTimer::GetStats().GetMin());
  REQUIRE(TestTimer::GetStats().GetTotal() >= TestTimer::GetStats().GetMax());
  REQUIRE(TestTimer::GetStats().GetLast() >= 0.0);

  TestTimer::ResetStats();
}

TEST_CASE("Test Timer Start and Stop", "[tools]")
{
  using TestTimer = emp::Timer<"Start Stop Timer Test">;

  TestTimer::ResetStats();

  {
    TestTimer timer;
    timer.Stop();
    REQUIRE(!timer.IsRunning());

    timer.Start();
    REQUIRE(timer.IsRunning());
    timer.Stop();
  }

  REQUIRE(TestTimer::GetStats().GetCount() == 2);
  REQUIRE(TestTimer::GetStats().GetAverage() >= 0.0);

  TestTimer::ResetStats();
}

TEST_CASE("Test Timer Pause and Unpause", "[tools]")
{
  using TestTimer = emp::Timer<"Pause Timer Test">;

  TestTimer::ResetStats();

  {
    TestTimer timer;
    timer.Pause();
    REQUIRE(timer.IsPaused());
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    timer.Stop();
  }

  REQUIRE(TestTimer::GetStats().GetCount() == 1);
  REQUIRE(TestTimer::GetStats().GetTotal() < 20.0);

  {
    TestTimer timer;
    timer.Pause();
    timer.Unpause();
    REQUIRE(!timer.IsPaused());
    timer.Stop();
  }

  REQUIRE(TestTimer::GetStats().GetCount() == 2);

  TestTimer::ResetStats();
}

TEST_CASE("Test Timer Stats Printing", "[tools]")
{
  using TestTimer = emp::Timer<"Printing Timer Test">;

  TestTimer::ResetStats();

  {
    TestTimer timer;
    timer.Stop();
  }

  std::ostringstream os;
  TestTimer::PrintStats(os);
  REQUIRE(os.str().find("Printing Timer Test") != std::string::npos);
  REQUIRE(os.str().find("total") != std::string::npos);

  TestTimer::ResetStats();
}
