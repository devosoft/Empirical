/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file events.cpp
 */

#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../UI/events.h"
#include "../../web/init.h"

void TestAlert() {
  emp::Alert("Page loaded!!");
}

int main() {

  emp::Initialize();

  emp::UI::OnDocumentReady(TestAlert);

}
