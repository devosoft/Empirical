#ifndef EMP_WEB_INIT_H
#define EMP_WEB_INIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Function to handle any special initialization for a web page (also calls base
//  emp::Initialize() function.
//

#include "../emtools/init.h"

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

}
}

#endif
