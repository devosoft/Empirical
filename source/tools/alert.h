/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  Alert.h
 *  @brief Define an Alert function that goes to std::cerr in c++ or to Alert() in Javascript.
 *  @note Status: RELEASE
 */

#ifndef EMP_ALERT_H
#define EMP_ALERT_H

#include "string_utils.h"

// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif


namespace emp {
#ifdef EMSCRIPTEN
  void Alert(const std::string & msg) { EM_ASM_ARGS({ msg = UTF8ToString($0); alert(msg); }, msg.c_str()); }
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

  /// A version of Alert that will cap how many times it can go off
  template <typename... TYPE_SET>
  static void CappedAlert(size_t cap, TYPE_SET... inputs) {
    static size_t cur_count = 0;
    if (cur_count++ < cap) Alert(emp::to_string(inputs...));
  }

  /// An object that will automatically output a message during construction or destruction,
  /// usually for use in debugging to disentangle timings.
  struct AlertObj {
    std::string msg;    ///< Message for the alert to print.
    bool on_construct;  ///< Should the message print automatically during construction?
    bool on_destruct;   ///< Should the message print automatically during destruction?

    AlertObj(const std::string & _msg, bool _on_c=true, bool _on_d=false)
    : msg(_msg), on_construct(_on_c), on_destruct(_on_d) { if (on_construct) emp::Alert(msg); }
    ~AlertObj() { if (on_destruct) emp::Alert(msg); }

    void Trigger() { emp::Alert(msg); }
    void SetMessage(const std::string & _msg) { msg = _msg; }
  };
}


#endif
