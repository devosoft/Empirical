/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file LoadingModal.cpp
 */

#include <iostream>

#include "emp/prefab/LoadingModal.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");
int main(){
  doc << "<p>Click button to show loading modal. It will close automatically after a few seconds.</p>";
  UI::Button loading_modal_demo([](){emscripten_run_script("DemoLoadingModal();");}, "Show Loading Modal");
  doc << loading_modal_demo;
  loading_modal_demo.SetAttr(
  "class", "btn btn-info"
  );
  emp::prefab::CloseLoadingModal();
}
