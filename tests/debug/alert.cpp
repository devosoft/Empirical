#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/debug/alert.hpp"

#include <sstream>
#include <iostream>

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
