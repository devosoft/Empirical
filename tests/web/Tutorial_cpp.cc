//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <string>

#include "web/Tutorial.h"

TEST_CASE("Test AddState/HasState", "[web][Tutorial]") {
  // Create a tutorial, it should not have state "test_state" at start
  // It should have the state "test_state" after we add it!
  Tutorial tut;
  REQUIRE(!tut.HasState("test_state"));
  tut.AddState("test_state");
  REQUIRE(tut.HasState("test_state"));
  REQUIRE(!tut.HasState("test_state_2"));
  tut.AddState("test_state_2");
  REQUIRE(tut.HasState("test_state_2"));
  REQUIRE(tut.HasState("test_state"));
}

TEST_CASE("Test StartAtState/Stop/IsActive/GetCurrentState", "[web][Tutorial]") {
  // Create a tutorial with two states
  // Tutorial should start as inactive with current state ""
  // StartAtState should change active status and the current state 
  
  // Setup
  Tutorial tut;
  REQUIRE(!tut.HasState("state_1"));
  REQUIRE(!tut.HasState("state_2"));
  tut.AddState("state_1");
  tut.AddState("state_2");
  REQUIRE(tut.HasState("state_1"));
  REQUIRE(tut.HasState("state_2"));
  REQUIRE(!tut.IsActive());
  REQUIRE(tut.GetCurrentState() == "");
  // Activate state_1
  tut.StartAtState("state_1");
  REQUIRE(tut.IsActive());
  REQUIRE(tut.GetCurrentState() == "state_1");
  // End tutorial
  tut.Stop();
  REQUIRE(!tut.IsActive());
  REQUIRE(tut.GetCurrentState() == "");
  // Re-activate, this time with state_2
  tut.StartAtState("state_2");
  REQUIRE(tut.IsActive());
  REQUIRE(tut.GetCurrentState() == "state_2");
  // Stop again
  tut.Stop();
  REQUIRE(!tut.IsActive());
  REQUIRE(tut.GetCurrentState() == "");
}
