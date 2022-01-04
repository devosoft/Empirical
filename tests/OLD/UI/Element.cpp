/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Element.cpp
 */

#include "../../jquery/jquery.h"

int main() {

  emp::JQ::Initialize();

  int test_num = 42;

  emp::JQ::document
    << "<h1>This is a test site!</h1>"
    << "<p>The value of test_num is " << test_num << ".</p>";

  emp::JQ::document.Update();
}
