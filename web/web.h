#ifndef EMP_WEB_H
#define EMP_WEB_H

// Include critical pieces of the web framework.
#include "Document.h"
#include "web_init.h"

namespace emp {
namespace web {

  // Some helper functions.
  // Live keyword means that whatever is passed in needs to be re-evaluated every update.
  namespace internal {

    // If a variable is passed in to live, construct a function to look up its current value.
    template <typename VAR_TYPE>
    std::function<std::string()> Live_impl(VAR_TYPE & var, bool) {
      return [&var](){ return emp::to_string(var); };
    }
    
    // If anything else is passed in, assume it is a function!
    template <typename IN_TYPE>
    std::function<std::string()> Live_impl(IN_TYPE && fun, int) {
      return [fun](){ return emp::to_string(fun()); };
    }
  }

  template <typename T>
  std::function<std::string()> Live(T && val) {
    return internal::Live_impl(std::forward<T>(val), true);
  }

}
}

#endif
