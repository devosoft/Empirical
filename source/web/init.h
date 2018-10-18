/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018.
 *
 *  @file  init.h
 *  @brief Define Initialize() and other functions to set up Empirical to build Emscripten projects.
 */

#ifndef EMP_INIT_H
#define EMP_INIT_H

/// If EMSCRIPTEN is defined, initialize everything.  Otherwise create useful stubs.
#ifdef EMSCRIPTEN

#include <emscripten.h>
#include "../tools/string_utils.h"

extern "C" {
  extern void EMP_Initialize();
}

namespace emp {

  /// Do all initializations for using EMP tricks with Emscripten.
  static void Initialize() {
    static bool init = false;      // Make sure we only initialize once!
    if (!init) {
      EMP_Initialize();   // Call JS initializations
      init = true;
    }
  }

  /// Setup timings on animations through Emscripten.
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
    namespace internal {
      /// If a variable is passed in to Live(), construct a function to look up its current value.
      template <typename VAR_TYPE>
      std::function<std::string()> Live_impl(VAR_TYPE & var, bool) {
        return [&var](){ return emp::to_string(var); };
      }

      /// If a non-variable is passed in to Live(), assume it is a function and print it each redraw.
      template <typename IN_TYPE>
      std::function<std::string()> Live_impl(IN_TYPE && fun, int) {
        return [fun](){ return emp::to_string(fun()); };
      }
    }

    /// Take a function or variable and set it up so that it can update each time a text box is redrawn.
    template <typename T>
    std::function<std::string()> Live(T && val) {
      return internal::Live_impl(std::forward<T>(val), true);
    }

    inline std::string ToJSLiteral(bool x) {
      if (x == true) return "true";
      else return "false";
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

  /// Stub for when Emscripten is not in use.
  static bool Initialize() {
    // Nothing to do here yet...
    // static_assert(false, "Emscripten web tools require emcc for compilation (for now).");
    return true;
  }

  /// Stub for when Emscripten is not in use.
  static bool InitializeAnim() {
    // Nothing to do here yet...
    return true;
  }

  namespace web {
    inline std::string ToJSLiteral(bool x) {
      if (x == true) return "true";
      else return "false";
    }
  }

}

#endif


#endif
