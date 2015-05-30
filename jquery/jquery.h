#ifndef EMP_JQUERY_H
#define EMP_JQUERY_H

#include "../emtools/init.h"

#include "ElementSlate.h"

namespace emp {
namespace JQ {

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

  template <typename VAR_TYPE>
  std::function<std::string()> Var(VAR_TYPE & var) {
    return [&var](){ return emp::to_string(var); };
  }

};
};

#endif
