//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Function to handle any special initialization for a web page (also calls base
//  emp::Initialize() function.

#ifndef EMP_WEB_INIT_H
#define EMP_WEB_INIT_H

#include "../tools/string_utils.h"
#include "init.h"

namespace emp {
namespace web {

  static bool Initialize() {

    // Make sure we only run Initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Run the base-level initialize in case it hasn't be run yet.
    emp::Initialize();

    // Setup the animation callback in Javascript
    EM_ASM({
        window.requestAnimFrame = (function(callback) {
            return window.requestAnimationFrame
              || window.webkitRequestAnimationFrame
              || window.mozRequestAnimationFrame
              || window.oRequestAnimationFrame
              || window.msRequestAnimationFrame
              || function(callback) { window.setTimeout(callback, 1000 / 60); };
          })();
      });

    return true;
  }

  // Some helper functions.
  // Live keyword means that whatever is passed in needs to be re-evaluated every update.
  namespace {
    // If a variable is passed in to live, construct a function to look up its current value.
    template <typename VAR_TYPE>
    std::function<std::string()> Live_impl(VAR_TYPE & var, bool) {
      return [&var](){ return emp::to_string(var); };
    }

    // If anything else is passed in, assume it is a function!
    template <typename IN_TYPE>
    std::function<std::string()> Live_impl(IN_TYPE && fun, int) {
      return [fun](){ return emp::to_string(fun()); };
    }
  }

  template <typename T>
  std::function<std::string()> Live(T && val) {
    return Live_impl(std::forward<T>(val), true);
  }
}
}

#endif
