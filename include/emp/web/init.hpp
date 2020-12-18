/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018.
 *
 *  @file  init.hpp
 *  @brief Define Initialize() and other functions to set up Empirical to build Emscripten projects.
 */

#ifndef EMP_INIT_H
#define EMP_INIT_H

#include <type_traits>

/// If __EMSCRIPTEN__ is defined, initialize everything.  Otherwise create useful stubs.
#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include "../tools/string_utils.hpp"

#ifdef  __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif //  __EMSCRIPTEN_PTHREADS__

extern "C" {
  extern void EMP_Initialize();
}

namespace emp {

  /// Setup timings on animations through Emscripten.
  static void InitializeAnim() {
    thread_local bool init = false;      // Make sure we only initialize once!
    if (!init) {
      // Setup the animation callback in Javascript
      MAIN_THREAD_EM_ASM({
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

  /// Add a listener on the browser thread that will look for incoming
  /// bitmaps and transfer them into web canvases.
  static void InitializeBitmapListener() {
    #ifdef __EMSCRIPTEN_PTHREADS__

    // adapted from https://stackoverflow.com/a/18002694
    if ( EM_ASM_INT({ // detect if we are a web worker
      return typeof WorkerGlobalScope !== 'undefined'
        && self instanceof WorkerGlobalScope;
    }) ) {
      MAIN_THREAD_EM_ASM({
        console.assert( Object.keys( PThread.pthreads ).length === 1 );
        Object.values(PThread.pthreads)[0].worker.addEventListener(
          'message',
          function( event ){
            if ( event.data.emp_canvas_id ) {
              document.getElementById(
                event.data.emp_canvas_id
              ).getContext("bitmaprenderer").transferFromImageBitmap( event.data.emp_bitmap
              );
            }
          }
        )
      });
    }

    #endif // __EMSCRIPTEN_PTHREADS__
  }

  /// Create a offscreen canvases registry that maps id to impl and a registry
  /// for updated canvases that need to be sent to the main thread.
  static void InitializeOffscreenCanvasRegistries() {
    #ifdef __EMSCRIPTEN_PTHREADS__

    // adapted from https://stackoverflow.com/a/18002694
    if ( EM_ASM_INT({ // detect if we are a web worker
      return typeof WorkerGlobalScope !== 'undefined'
        && self instanceof WorkerGlobalScope;
    }) ) EM_ASM({
      emp_i.offscreen_canvases = {};
      emp_i.pending_offscreen_canvas_ids = new Set();
    });

    #endif // __EMSCRIPTEN_PTHREADS__
  }

  /// globalThis polyfill to provide globalThis support in older environments
  /// adapted from https://mathiasbynens.be/notes/globalthis
  static void SetupGlobalThisPolyfill() {
    EM_ASM({
      (function() {
        if (typeof globalThis === 'object') return;
        Object.prototype.__defineGetter__('__magic__', function() {
          return this;
        });
        __magic__.globalThis = __magic__; // lolwat
        delete Object.prototype.__magic__;
      }());
    });
  }

  /// Do all initializations for using EMP tricks with Emscripten.
  static void Initialize() {

    SetupGlobalThisPolyfill();

    // have to dip into javascript because static and thread_local are wonky
    // with pthreads
    const bool should_run = EM_ASM_INT({
      if ( !globalThis.emp_init_once_flag ) {
        globalThis.emp_init_once_flag = true;
        return true;
      } else return false;
    });

    if ( should_run ) {
      EMP_Initialize();   // Call JS initializations
      InitializeAnim();

      #ifdef __EMSCRIPTEN_PTHREADS__
      MAIN_THREAD_EM_ASM({ _EMP_Initialize(); });
      InitializeBitmapListener();
      InitializeOffscreenCanvasRegistries();
      #endif

    }
  }


  namespace web {
    // Some helper functions.
    // Live keyword means that whatever is passed in needs to be re-evaluated every update.
    namespace internal {
      /// If a variable is passed in to Live(), construct a function to look up its current value.
      template <typename VAR_TYPE>
      std::function<std::string()> Live_impl(VAR_TYPE & var, int) {
        return [&var](){ return emp::to_string(var); };
      }

      /// If a non-variable is passed in to Live(), assume it is a function and print it each redraw.
      template <
        typename IN_TYPE,
        typename = std::enable_if_t< std::is_invocable<IN_TYPE>::value >
      >
      std::function<std::string()> Live_impl(IN_TYPE && fun, bool) {
        return [fun](){ return emp::to_string(fun()); };
      }
    }

    /// Take a function or variable and set it up so that it can update each time a text box is redrawn.
    template <typename T>
    std::function<std::string()> Live(T && val) {
      return internal::Live_impl(std::forward<T>(val), bool{});
    }

    inline std::string ToJSLiteral(bool x) {
      if (x == true) return "true";
      else return "false";
    }
  }

}


// === Initialization for NON-emscripten to ignore macros ===

#else

#define MAIN_THREAD_EM_ASM(...)
#define MAIN_THREAD_EM_ASM(...)
#define MAIN_THREAD_EM_ASM_INT(...) 0
#define MAIN_THREAD_EM_ASM_DOUBLE(...) 0.0
#define MAIN_THREAD_EM_ASM_INT_V(...) 0
#define MAIN_THREAD_EM_ASM_DOUBLE_V(...) 0.0

#include <fstream>

namespace emp {
  std::ofstream debug_file("debug_file");

  /// Stub for when Emscripten is not in use.
  static bool Initialize() {
    // Nothing to do here yet...
    static_assert(false, "Emscripten web tools require emcc for compilation (for now).");
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
