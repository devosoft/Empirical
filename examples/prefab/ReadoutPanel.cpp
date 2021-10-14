/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file ReadoutPanel.cpp
 */

#include "emp/math/Random.hpp"
#include "emp/prefab/ReadoutPanel.hpp"
#include "emp/web/Button.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int counter = 0;

int main() {

  emp::prefab::ReadoutPanel values{"Readout Values", 100}; // Refreshes 10 times a second

  // A random number generator
  std::function<std::string()> random_number = [=]() mutable {
    return emp::to_string(std::rand());
  };
  values.AddValues(
    "Random", "A randomly generated number", random_number,
    "Counter", "How many times you've clicked the button", counter
  );

  doc << values;
  UI::Button adder{[](){ ++counter; }, "Add one to counter"};
  adder.SetAttr("class", "btn btn-primary");
  doc << adder;
}
