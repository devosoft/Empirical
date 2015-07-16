#ifndef EMP_INIT_H
#define EMP_INIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains the emp::Initialize() function, which should usually be run when
//  the Empirical library is in use to build an Emscripten project.
//

#include <emscripten.h>

extern "C" {
  extern void EMP_Initialize();
}

namespace emp {

  static bool Initialize() {

    // Make sure we only initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Setup everything that needs to be initialized on the JS side...
    EMP_Initialize();

    return true;
  };

};

#endif
