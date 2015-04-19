#ifndef EMP_ALERT_H
#define EMP_ALERT_H

//////////////////////////////////////////////////////////////////////////////////////////
// 
// This file sets up an Alert function that will go to std::cerr in c++ or
// to Alert() in Javascript.
//


#include "string_utils.h"

// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif


namespace emp {
#ifdef EMSCRIPTEN
  void Alert(const std::string & msg) { EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str()); }
#else
  void Alert(const std::string & msg) { std::cerr << msg << std::endl; }
#endif
  template <typename... TYPE_SET>
  void Alert(TYPE_SET... inputs) { Alert(emp::to_string(inputs...)); }
};




#endif
