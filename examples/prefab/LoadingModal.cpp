//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/web/web.hpp"
#include "emp/prefab/LoadingModal.hpp"

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
