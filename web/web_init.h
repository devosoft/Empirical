//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Function to handle any special initialization for a web page (also calls base
//  emp::Initialize() function.

#ifndef EMP_WEB_INIT_H
#define EMP_WEB_INIT_H

#include "../tools/string_utils.h"
#include "init.h"

namespace emp {
namespace web {

  static bool Initialize() {

    // Make sure we only run Initialize once!
    static bool init = false;
    if (init) return false;
    init = true;

    // Run the base-level initialize in case it hasn't be run yet.
    emp::Initialize();

    return true;
  }

}
}

#endif
