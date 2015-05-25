#ifndef EMP_JQ_EVENTS_H
#define EMP_JQ_EVENTS_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Event handlers that use JQuerry.
//

#include "../emtools/html5_events.h"

namespace emp {
namespace JQ {

  template<typename FUN_TYPE> void OnDocumentReady(FUN_TYPE && fun) {
    EM_ASM_ARGS({  $( document ).ready(function() { emp.Callback($0); });  }, JSWrapOnce(fun));
  }
 
};
};

#endif
