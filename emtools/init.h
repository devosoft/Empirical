#ifndef EMP_INIT_H
#define EMP_INIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains the emp::Initialize() function, which should usually be run when
//  the Empirical library is in use to build an Emscripten project.
//

// If EMSCRIPTEN is defined, initialize everything.  Otherwise create useful stubs.

#ifdef EMSCRIPTEN

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
  }

}

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

}

#endif


#endif
