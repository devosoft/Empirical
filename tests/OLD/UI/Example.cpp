/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Example.cpp
 */

#include "../../UI/UI.h"
#include "../../web/emfunctions.h"

namespace UI = emp::UI;
UI::Document doc("emp_base");

int main() {

  UI::Initialize();

  doc << "<h1>This is my header!</h1>"
      << "This is some text...";

  doc.Update();
}
