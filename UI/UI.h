#ifndef EMP_UI_H
#define EMP_UI_H

#include "../emtools/init.h"

#include "ElementSlate.h"
#include "UI_base.h"

namespace emp {
namespace UI {

  static ElementSlate document("emp_base");

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
