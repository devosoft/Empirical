#ifndef TUBE_WEB_H
#define TUBE_WEB_H

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

#endif
