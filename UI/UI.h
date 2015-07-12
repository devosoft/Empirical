#ifndef EMP_UI_H
#define EMP_UI_H

#include "../emtools/init.h"

#include "ElementSlate.h"

namespace emp {
namespace UI {

  static Document document("emp_base");

  bool Initialize() {

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

};
};

#endif
