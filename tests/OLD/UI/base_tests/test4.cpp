/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file test4.cpp
 */

#include <emscripten.h>
#include <iostream>

#include "../../jquery/JQManager.h"

int main() {
  emp::JQManager wm("emp_base");
  emp::JQElement * we = new emp::JQElement("mytext");
  wm.AddFront(*we);
  we->SetText("<p>This is a <b>TEST</b></p>");
}
