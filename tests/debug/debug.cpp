/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file debug.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/debug/debug.hpp"

TEST_CASE("Test debug", "[debug]")
{
  EMP_TRACK_LINE("Test1");
  EMP_TRACK_LINE("Test2");
  EMP_TRACK_LINE("Test3");

  CHECK(emp::GetDebugLine("Test1") == "debug.cpp:15");
  CHECK(emp::GetDebugLine("Test2") == "debug.cpp:16");
  CHECK(emp::GetDebugLine("Test3") == "debug.cpp:17");

  EMP_TRACK_LINE("Test4");

  // Make sure they all work a second time and test the new one...
  CHECK(emp::GetDebugLine("Test1") == "debug.cpp:15");
  CHECK(emp::GetDebugLine("Test2") == "debug.cpp:16");
  CHECK(emp::GetDebugLine("Test3") == "debug.cpp:17");
  CHECK(emp::GetDebugLine("Test4") == "debug.cpp:23");
}
