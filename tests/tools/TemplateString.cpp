/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2026
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <string_view>

#include "emp/tools/TemplateString.hpp"

template <emp::TemplateString NAME>
struct NamedThing {
  static constexpr std::string_view GetName() { return NAME.AsStringView(); }
  static constexpr size_t GetSize() { return NAME.size(); }
};

TEST_CASE("Test TemplateString basics", "[tools]")
{
  constexpr emp::TemplateString name{"Example"};

  REQUIRE(name.size() == 7);
  REQUIRE(name.AsStringView() == "Example");
  REQUIRE(name.data()[7] == '\0');
}

TEST_CASE("Test TemplateString as template parameter", "[tools]")
{
  using ExampleThing = NamedThing<"ExampleThing">;

  REQUIRE(ExampleThing::GetName() == "ExampleThing");
  REQUIRE(ExampleThing::GetSize() == 12);
}
