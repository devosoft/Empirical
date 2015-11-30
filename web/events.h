//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Event handlers that use JQuerry.
//
//  OnDocumentReady(fun)
//    Runs the specified function when the document is finished loading and being set up.
//

#ifndef EMP_WEB_EVENTS_H
#define EMP_WEB_EVENTS_H

#include <utility>

#include "../emtools/html5_events.h"

namespace emp {
namespace web {

  template <typename FUN_TYPE> void OnDocumentReady(FUN_TYPE && fun) {
    // const int fun_id = JSWrapOnce(fun);
    const int fun_id = JSWrap(std::forward<FUN_TYPE>(fun), "", true);
    EM_ASM_ARGS({  $( document ).ready(function() { emp.Callback($0); });  }, fun_id);
  }
 
}
}

#endif
