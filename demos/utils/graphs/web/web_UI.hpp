/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file web_UI.hpp
 *
 */

#ifndef DEMOS_UTILS_GRAPHS_WEB_WEB_UI_HPP_INCLUDE
#define DEMOS_UTILS_GRAPHS_WEB_WEB_UI_HPP_INCLUDE

#include <emscripten.h>

class VM_UI_base {
protected:
  cHardware * hardware;

public:
  VM_UI_base() : hardware(NULL) { ; }
  virtual ~VM_UI_base() { ; }

  void UpdateCode() {
    std::string inst_bg = "#f0f0f0";

    std::stringstream ss;
    ss << "<table style=\"background-color:" << inst_bg << ";\">"
       << "<tr><th>Line</th><th>Instruction</th><th>Arg 1</th><th>Arg 2</th><th>Arg 3</th></tr>";

    EM_ASM_ARGS({
        var code = UTF8ToString($0);
        var code_obj = document.getElementById("code");
        code_obj.innerHTML = code;
    }, ss.str().c_str());
  }

};

#endif // #ifndef DEMOS_UTILS_GRAPHS_WEB_WEB_UI_HPP_INCLUDE
