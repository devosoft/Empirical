#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include "config/config_web_interface.h"
#include "assets/config_setup.h"

TEST_CASE("Test config_web_interface", "[config]")
{
  MyConfig config;
  emp::ConfigWebUI ui(config);
}
