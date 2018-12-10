#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/alert.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Alert", "[tools]")
{ 
  std::stringstream buffer;
  std::streambuf* cerrStream = std::cerr.rdbuf(buffer.rdbuf());
  
  // basic Alert
  emp::Alert("Whoops! Try again.");
  REQUIRE( (buffer.str() == "Whoops! Try again.\n") );
  buffer.clear();
  buffer.str(std::string());
  
  // AlertObj
  emp::AlertObj an_alert("ALERT!",true,true);
  REQUIRE( (buffer.str() == "ALERT!\n") );
  buffer.clear();
  buffer.str(std::string());
  
  an_alert.SetMessage("ALERT x2!");
  an_alert.Trigger();
  REQUIRE( (buffer.str() == "ALERT x2!\n") );
  buffer.clear();
  buffer.str(std::string());
  
  an_alert.SetMessage("DESTROYED");
  an_alert.~AlertObj();
  REQUIRE( (buffer.str() == "DESTROYED\n") );
  buffer.clear();
  buffer.str(std::string());  
  
  // CappedAlert
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  emp::CappedAlert(2,"Hello!");
  REQUIRE( (buffer.str() == "Hello!\nHello!\n") );
  buffer.clear();
  buffer.str(std::string());  
  
  // Templated Alert
  emp::Alert(5," is a good number, but ",7," is even better.");
  REQUIRE( (buffer.str() == "5 is a good number, but 7 is even better.\n") );
  buffer.clear();
  buffer.str(std::string()); 
}