//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file contains the emp::Initialize() function, which should usually be run when
//  the Empirical library is in use to build an Emscripten project.

#ifndef EMP_INIT_H
#define EMP_INIT_H

// If EMSCRIPTEN is defined, initialize everything.  Otherwise create useful stubs.
#ifdef EMSCRIPTEN

#include <emscripten.h>
#include "../tools/string_utils.h"

extern "C" {
  extern void EMP_Initialize();
}

namespace emp {

  static void Initialize() {
    static bool init = false;      // Make sure we only initialize once!
    if (!init) EMP_Initialize();   // Call JS initializations
    init = true;
  }

  static void InitializeAnim() {
    static bool init = false;      // Make sure we only initialize once!
    if (!init) {
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
    }

    init = true;
  }

  namespace web {
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


// === Initialization for NON-emscripten to ignore macros ===

#else

#define EM_ASM(...)
#define EM_ASM_ARGS(...)
#define EM_ASM_INT(...) 0
#define EM_ASM_DOUBLE(...) 0.0
#define EM_ASM_INT_V(...) 0
#define EM_ASM_DOUBLE_V(...) 0.0

#include <fstream>

namespace emp {
  std::ofstream debug_file("debug_file");

  static bool Initialize() {
    // Nothing to do here yet...
    return true;
  }

  static bool InitializeAnim() {
    // Nothing to do here yet...
    return true;
  }


}

#endif


#endif
