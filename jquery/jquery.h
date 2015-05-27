#ifndef EMP_JQUERY_H
#define EMP_JQUERY_H

#include "../emtools/init.h"

#include "ElementGroup.h"

namespace emp {
namespace JQ {

  static ElementGroup document("document");

  bool Initialize() {

    // Make sure we only run Initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Run the base-level initialize in case it hasn't be run yet.
    emp::Initialize();

    return true;
  }

};
};

#endif
