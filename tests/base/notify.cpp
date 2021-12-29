//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2021.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/notify.hpp"
#include "emp/base/vector.hpp"

#include <iostream>
#include <sstream>
#include <string>

TEST_CASE("Test notifications", "[base]")
{
  // setup streams to use to capture notification output.
  emp::vector<std::string> message_results;
  emp::vector<std::string> warning_results;
  emp::vector<std::string> error_results;
  emp::vector<std::string> exception_results;
  emp::vector<std::string> special_results;

  emp::notify::MessageHandlers().Add([&message_results](const std::string & msg){ message_results.push_back(msg); return true; });
  emp::notify::WarningHandlers().Add([&warning_results](const std::string & msg){ warning_results.push_back(msg); return true; });
  emp::notify::ErrorHandlers().Add([&error_results](const std::string & msg){ error_results.push_back(msg); return true; });
  emp::notify::AddHandler(
    [&exception_results](const std::string & id, const std::string & /*msg*/){
      exception_results.push_back(id); return true;
    }
  );

  size_t special_count = 0;
  emp::notify::AddHandler(
    "PASS",
    [&special_results, &special_count](const std::string & id, const std::string & /*msg*/) {
      special_results.push_back(id);
      ++special_count;
      return true;
    }
  );

  emp::notify::AddHandler(
    "FAIL",
    [&special_results, &special_count](const std::string & id, const std::string & /*msg*/) {
      special_results.push_back(id);
      ++special_count;
      return false;
    }
  );

  size_t exit_count = 0;
  emp::notify::ReplaceExitHandlers( [&exit_count](size_t exit_code){ ++exit_count; (void) exit_code; } );

  CHECK(message_results.size() == 0);
  emp::notify::Message("Message1");
  CHECK(message_results.size() == 1);
  CHECK(message_results.back() == "Message1");
  emp::notify::Message("Message2");
  CHECK(message_results.size() == 2);
  CHECK(message_results.back() == "Message2");

  emp::notify::Warning("This is Warning1");
  emp::notify::Warning("Warning2");
  emp::notify::Warning("Warning3");
  CHECK(warning_results.size() == 3);
  CHECK(warning_results.back() == "Warning3");

  emp::notify::Error("ERROR!!!");
  CHECK(error_results.size() == 1);
  CHECK(error_results.back() == "ERROR!!!");

  emp::notify::Exception("UNKNOWN", "This is a first test of an unknown exception.");
  emp::notify::Exception("PASS", "This is an exception that will be repaired.");
  emp::notify::Exception("FAIL", "This is an exception that will NOT be repaired.");
  emp::notify::Exception("FAIL", "This one won't be repaired either.");
  emp::notify::Exception("UNKNOWN", "This is the first unknown expression happening again.");
  emp::notify::Exception("UNKNOWN2", "This is a brand new unknown expression.");
  emp::notify::Exception("UNKNOWN", "This is the original unknown expression once again.");

  CHECK(exception_results.size() == 6); // Only unknown and FAIL expressions should end up here.
  CHECK(special_results.size() == 3);   // Both PASS and FAIL should be found here.
  CHECK(exception_results.back() == "UNKNOWN");
  CHECK(special_results.back() == "FAIL");


  double x = 0.0;
  emp::notify::AddHandler(
    "INC",
    [&x](const std::string & /*id*/, const std::string & /*msg*/) { x+=1.0; return true; }
  );
  emp::notify::AddHandler(
    "DOUBLE",
    [&x](const std::string & /*id*/, const std::string & /*msg*/) { x*=2.0; return true; }
  );

  emp::notify::Exception("DOUBLE");
  CHECK(x == 0.0);
  emp::notify::Exception("INC");
  CHECK(x == 1.0);
  emp::notify::Exception("DOUBLE");
  CHECK(x == 2.0);

  emp::notify::Pause();  // Stop actually running notifications!

  emp::notify::Exception("INC");
  CHECK(x == 2.0);
  emp::notify::Exception("INC");
  CHECK(x == 2.0);
  emp::notify::Exception("DOUBLE");
  CHECK(x == 2.0);
  emp::notify::Exception("DOUBLE");
  CHECK(x == 2.0);

  emp::notify::Unpause();  // Turn notifications back on; should occur in the correct order!
  CHECK(x == 16.0);
  
}
