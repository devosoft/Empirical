/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file alert.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/debug/alert.hpp"

TEST_CASE("Test Alert", "[debug]")
{

  // basic Alert
  emp::Alert("Whoops! Try again.");

  // AlertObj
  emp::AlertObj an_alert("ALERT!",true,true);

  an_alert.SetMessage("ALERT x2!");
  an_alert.Trigger();

  an_alert.SetMessage("DESTROYED");
  an_alert.~AlertObj();

  // CappedAlert
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");

  // Templated Alert
  emp::Alert(5," is a good number, but ",7," is even better.");
}
