//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <string>

#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/web/color_map.hpp"

TEST_CASE("Test GetHueMap", "[web][color_map]") {
  // Create a hue map with 30 colors with default values
  // - h: 0.0:360.0
  // - s: 100%
  // - l: 50%
  const auto & test_map = emp::GetHueMap(30);
  REQUIRE(test_map.size() == 30);
  for (size_t i = 0; i < test_map.size(); ++i) {
    std::string color_str(test_map[i]);
    std::string prefix(emp::string_pop(color_str, "("));
    REQUIRE(prefix == "hsl");
    double h = emp::from_string<double>(emp::string_pop(color_str, ","));
    REQUIRE(h >= 0); //
    REQUIRE(h <= 360);
    std::string s(emp::string_pop(color_str, ","));
    REQUIRE(s == "100%"); // Default value
    std::string l(emp::string_pop(color_str, ")"));
    REQUIRE(l == "50%"); // Default value
  }

  // Create a hue map with 60 colors with default values
  // - h: 8:100
  // - s: 2%
  // - l: 98%
  const auto & test_map2 = emp::GetHueMap(60, 8, 100, 2, 98);
  REQUIRE(test_map2.size() == 60);
  for (size_t i = 0; i < test_map2.size(); ++i) {
    std::string color_str(test_map2[i]);
    std::string prefix(emp::string_pop(color_str, "("));
    REQUIRE(prefix == "hsl");
    double h = emp::from_string<double>(emp::string_pop(color_str, ","));
    REQUIRE(h >= 8); //
    REQUIRE(h <= 100);
    std::string s(emp::string_pop(color_str, ","));
    REQUIRE(s == "2%"); // Default value
    std::string l(emp::string_pop(color_str, ")"));
    REQUIRE(l == "98%"); // Default value
  }
}

TEST_CASE("Test GetHSLMap", "[web][color_map]") {
  const auto & hsl_map30 = emp::GetHSLMap(30);
  const auto & hue_map30 = emp::GetHueMap(30);
  REQUIRE(hsl_map30.size() == 30);
  REQUIRE(hsl_map30 == hue_map30); // Given default argument values, these should be identical.


  const auto & hsl_map60 = emp::GetHSLMap(
    60,      // # colors
    27, 102, // h range
    1, 99,   // s range
    42, 43   // l range
  );
  REQUIRE(hsl_map60.size() == 60);
  for (size_t i = 0; i < hsl_map60.size(); ++i) {
    std::string color_str(hsl_map60[i]);
    std::string prefix(emp::string_pop(color_str, "("));
    REQUIRE(prefix == "hsl");
    double h = emp::from_string<double>(emp::string_pop(color_str, ","));
    REQUIRE(h >= 27);
    REQUIRE(h <= 102);
    std::string s_percent(emp::string_pop(color_str, ","));
    double s = emp::from_string<double>(
      emp::string_pop(s_percent, "%")
    );
    REQUIRE(s >= 1);
    REQUIRE(s <= 99);
    std::string l_percent(emp::string_pop(color_str, ")"));
    double l = emp::from_string<double>(
      emp::string_pop(l_percent, "%")
    );
    REQUIRE(l >= 42);
    REQUIRE(l <= 43);
  }
}

TEST_CASE("Test ColorRGB", "[web][color_map]") {
  REQUIRE(emp::ColorRGB(255,255,255) == "#ffffff");
  REQUIRE(emp::ColorRGB(0,0,0) == "#000000");
  REQUIRE(emp::ColorRGB(0,255,0) == "#00ff00");
  REQUIRE(emp::ColorRGB(26,28,147) == "#1a1c93");
  REQUIRE(emp::ColorRGB(175,175,144) == "#afaf90");
  REQUIRE(emp::ColorRGB(195,30,204) == "#c31ecc");
  REQUIRE(emp::ColorRGB(195,0,204) == "#c300cc");
  REQUIRE(emp::ColorRGB(195,0,255) == "#c300ff");

  REQUIRE(emp::ColorRGB(255,255,255,0.5) == "rgba(255,255,255,0.5)");
  REQUIRE(emp::ColorRGB(255,255,255,1.0) == "rgba(255,255,255,1)");
  REQUIRE(emp::ColorRGB(26,28,147,0.5) == "rgba(26,28,147,0.5)");
  REQUIRE(emp::ColorRGB(26,28,147,1.0) == "rgba(26,28,147,1)");
}

TEST_CASE("Test ColorHSL", "[web][color_map]") {
  REQUIRE(emp::ColorHSL(198,100,100) == "hsl(198,100%,100%)");
  REQUIRE(emp::ColorHSL(18,0,10) == "hsl(18,0%,10%)");
  REQUIRE(emp::ColorHSL(360,6,10) == "hsl(360,6%,10%)");
  REQUIRE(emp::ColorHSL(0,6,10) == "hsl(0,6%,10%)");
}

TEST_CASE("Test ColorHSV", "[web][color_map]") {
  REQUIRE(emp::ColorHSV(0,0,1.0) == "#ffffff");
  REQUIRE(emp::ColorHSV(99.0,0,1.0) == "#ffffff");
  REQUIRE(emp::ColorHSV(360.0,0,1.0) == "#ffffff");
  REQUIRE(emp::ColorHSV(0,0,0) == "#000000");
  REQUIRE(emp::ColorHSV(0.3333*360,1.0,1.0) == "#00ff00");
  REQUIRE(emp::ColorHSV(0.6639*360,0.8231,0.5765) == "#1a1c93");
  REQUIRE(emp::ColorHSV(0.1667*360,0.1771,0.6863) == "#afaf90");
  REQUIRE(emp::ColorHSV(297,0.85,0.8000) == "#c31ecc");
  REQUIRE(emp::ColorHSV(0.8260*360,1.0,0.8000) == "#c300cc");
  REQUIRE(emp::ColorHSV(286,1.0,1.0) == "#c300ff");
}
