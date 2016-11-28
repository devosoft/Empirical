//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
// This file sets up an Alert function that will go to std::cerr in c++ or
// to Alert() in Javascript.

#ifndef EMP_ALERT_H
#define EMP_ALERT_H

#include "string_utils.h"

// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif


namespace emp {
#ifdef EMSCRIPTEN
  void Alert(const std::string & msg) { EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str()); }
#else
  /// Send msg to cerr if in C++, or show msg in an alert box if compiled to Javascript
  /// Input can be any number of arguments of any types as long as the can be converted to
  /// strings with emp::to_string().
  void Alert(const std::string & msg) { std::cerr << msg << std::endl; }
#endif
  /// @cond TEMPLATES
  template <typename... TYPE_SET>
  void Alert(TYPE_SET... inputs) { Alert(emp::to_string(inputs...)); }
  /// @endcond

  template <typename... TYPE_SET>
  /** A version of Alert that will cap how many times it can go off */
  static void CappedAlert(int cap, TYPE_SET... inputs) {
    static int cur_count = 0;
    if (cur_count++ < cap) Alert(emp::to_string(inputs...));
  }
}


#endif
