#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "config/SettingConfig.h"

TEST_CASE("Test SettingConfig", "[config]")
{
    emp::SettingConfig config_set;
    config_set.AddSetting<int>("test_non_combo") = 5;  

    CHECK(config_set.GetValue<int>("test_non_combo") == 5); 

}
