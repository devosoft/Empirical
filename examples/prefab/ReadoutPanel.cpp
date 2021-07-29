//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2021.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/math/Random.hpp"
#include "emp/prefab/Card.hpp"
#include "emp/web/Button.hpp"
#include "emp/web/web.hpp"


namespace UI = emp::web;

UI::Document doc("emp_base");

int counter = 0;

int main() {

  emp::prefab::ReadoutPanel values("Readout Values", 100); // Refreshes 10 times a second

  // A random number generator
  std::function<std::string()> random_number = [=]() mutable {
    static emp::Random rand;
    return emp::to_string(rand.GetUInt());
  };
  values.AddValues(
    "Random", "A randomly generated number", random_number,
    "Counter", "How many times you've clicked the button", counter
  );

  doc << values;
  doc << UI::Button([](){ ++counter; }, "Add one to counter");
}
