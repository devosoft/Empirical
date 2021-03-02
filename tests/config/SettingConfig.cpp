#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/vector_utils.hpp"
#include "emp/config/SettingConfig.hpp"

TEST_CASE("Test SettingConfig", "[config]")
{
emp::SettingConfig config;

config.AddSetting<int>("num_runs") = 200;
config.AddComboSetting<int>("pop_size") = { 100,200,400,800 };

// Test setup
emp::vector<std::string> setting_names = config.GetSettingMapNames();
CHECK(emp::Has(setting_names, "num_runs"));
CHECK(emp::Has(setting_names, "pop_size"));
auto setting_ptrs = config.GetSettingMapBase();
CHECK(setting_names.size() == 2);

// Test getting values
CHECK(config.GetValue<int>("num_runs") == 200);
CHECK(config.GetValue<int>("pop_size") == 100);
CHECK(config.MaxValue<int>("pop_size") == 800);

// Test parsing
emp::vector<std::string> opts = {"example_prog", "example_unused", "--pop_size", "800,900,1000", "--num_runs", "500"};
CHECK(config.ProcessOptions(opts));
CHECK(config.HasUnusedArgs());
CHECK(config.GetUnusedArgs()[0] == "example_unused");
CHECK(config.GetExeName() == "example_prog");
CHECK(!config.HasErrors());
CHECK(config.CurSettings() == "500,800");
CHECK(config.CountCombos() == 3);
CHECK(config.CurComboString() == "800");
CHECK(config.GetComboHeaders() == "pop_size");
CHECK(config.GetSettingHeaders() == "num_runs,pop_size");
CHECK(config.GetValue<int>("num_runs") == 500);
CHECK(config.GetValue<int>("pop_size") == 800);

// Test copy constructor
emp::SettingConfig other_config(config);
CHECK(other_config.HasUnusedArgs());
CHECK(other_config.GetUnusedArgs()[0] == "example_unused");
CHECK(other_config.GetExeName() == "example_prog");
CHECK(!other_config.HasErrors());
CHECK(other_config.CurSettings() == "500,800");
CHECK(other_config.CountCombos() == 3);
CHECK(other_config.CurComboString() == "800");
CHECK(other_config.GetComboHeaders() == "pop_size");
CHECK(other_config.GetSettingHeaders() == "num_runs,pop_size");
CHECK(other_config.GetValue<int>("num_runs") == 500);
CHECK(other_config.GetValue<int>("pop_size") == 800);

// Test getting next combo
config.NextCombo();
CHECK(config.CurSettings() == "500,900");

// Test actions
config.AddAction("test_action", "a test action", 'a', [](){std::cout << "An action happened!" << std::endl;});

// Test printing help
config.PrintHelp();

}