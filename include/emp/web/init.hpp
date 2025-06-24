/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/init.hpp
 * @brief Define Initialize() and other functions to set up Empirical to build Emscripten projects.
 *
 * This file should be included if you are compiling Empirical's web tools with Emscripten.
 * It ensures that behind the scenes stuff is all set up properly. It also defines some
 * useful stubs and dummy functions so that your code will still be possible to compile with a normal
 * C++ compiler (although the web part won't do anything, of course). These stubs are also helpful
 * for avoiding confusion in linters and IDEs.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_INIT_HPP_GUARD
#define INCLUDE_EMP_WEB_INIT_HPP_GUARD

#ifndef DOXYGEN_SHOULD_SKIP_THIS  // This file is just going to confuse doxygen

#include <type_traits>

#include "../base/assert_warning.hpp"
#include "../base/compile_consts.hpp"
#include "../tools/string_utils.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>
#endif  // #ifdef __EMSCRIPTEN_PTHREADS__

extern "C" {
  extern void EMP_Initialize();
}

namespace emp {

  /// Setup timings on animations through Emscripten.
  static void InitializeAnim() {
    thread_local bool init = false;  // Make sure we only initialize once!
    if (!init) {
      // clang-format off
      // Setup the animation callback in Javascript
      MAIN_THREAD_EM_ASM({
        window.requestAnimFrame = (function(callback) {
            return window.requestAnimationFrame
              || window.webkitRequestAnimationFrame
              || window.mozRequestAnimationFrame
              || window.oRequestAnimationFrame
              || window.msRequestAnimationFrame
              || function(callback) { window.setTimeout(callback, 1000 / 60); }
        })();
      });
      // clang-format on
    }

    init = true;
  }

  /// Add a listener on the browser thread that will look for incoming
  /// bitmaps and transfer them into web canvases.
  static void InitializeBitmapListener() {
    if constexpr (emp::compile::EMSCRIPTEN_PTHREADS) {
      // adapted from https://stackoverflow.com/a/18002694
      // clang-format off
      if (EM_ASM_INT({  // detect if we are a web worker
            return typeof WorkerGlobalScope !== 'undefined' && self instanceof WorkerGlobalScope;
          })) {
        MAIN_THREAD_EM_ASM({
          console.assert(Object.keys(PThread.pthreads).length === 1);
          Object.values(PThread.pthreads)[0].worker.addEventListener(
            'message',
            function(event) {
              if (event.data.emp_canvas_id) {
                document.getElementById(event.data.emp_canvas_id)
                  .getContext("bitmaprenderer")
                  .transferFromImageBitmap(event.data.emp_bitmap);
              }
            })
        });
      }
      // clang-format on
    }

  }

  /// Create an offscreen canvases registry that maps id to impl and a registry
  /// for updated canvases that need to be sent to the main thread.
  static void InitializeOffscreenCanvasRegistries() {
    if constexpr (emp::compile::EMSCRIPTEN_PTHREADS) {
      // adapted from https://stackoverflow.com/a/18002694
      // clang-format off
      if (EM_ASM_INT({  // detect if we are a web worker
            return typeof WorkerGlobalScope !== 'undefined' &&
                  self instanceof WorkerGlobalScope;
          })) {
        EM_ASM({
          emp_i.offscreen_canvases = {};
          emp_i.pending_offscreen_canvas_ids = new Set();
        });
      }
      // clang-format on
    }
  }

  /// Do all initializations for using EMP tricks with Emscripten.
  static void Initialize() {
    // Dip into Javascript because static and thread_local is wonky with pthreads
    const bool should_run = EM_ASM_INT({
      if (globalThis.emp_init_once_flag) return false;
      return (globalThis.emp_init_once_flag = true);
    });

    if (should_run) {
      EMP_Initialize();  // Call JS initializations for current thread.
      InitializeAnim();

      if constexpr (emp::compile::EMSCRIPTEN_PTHREADS) {
        // Make sure _EMP_Initialize has been run in the main thread.
        // Note: EMP_Initialize has the underscore prepended in JS code.
        MAIN_THREAD_EM_ASM({ _EMP_Initialize(); });
        InitializeBitmapListener();
        InitializeOffscreenCanvasRegistries();
      }
    }
  }

  namespace web {
    /// Take a function or variable and set it up so that it can update each time a text box is redrawn.
    template <typename T>
    std::function<std::string()> Live(T && val) {
      // If the value passed in is a function, call it and convert the result to a string.
      if constexpr (std::is_invocable<T>::value) {
        return [val]() { return emp::to_string(val()); };
      }

      // Otherwise directly convert its current value to a string.
      else return [&val]() { return emp::to_string(val); };
    }

    constexpr inline std::string ToJSLiteral(bool x) { return x ? "true" : "false"; }
  }
}

#endif  // #ifdef __EMSCRIPTEN__

#endif  // This file is just going to confuse doxygen : #ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif  // #ifndef INCLUDE_EMP_WEB_INIT_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: lolwat globalthis mathiasbynens bitmaprenderer moz
