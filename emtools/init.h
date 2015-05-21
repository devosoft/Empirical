#ifndef EMP_INIT_H
#define EMP_INIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This file contains the emp::Initialize() function, which should usually be run when
//  the Empirical library is in use to build an Emscripten project.
//

#include <emscripten.h>

namespace emp {

  bool Initialize() {

    // Setup the empCppCallback function to be called from Javascript.
    EM_ASM({  empCppCallback = Module.cwrap('empCppCallback', null, ['number']);  });

    return true;
  };

};

#endif
