//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file is the main file to include the entire Empirical web framework.
//
//  The main to includes are Document.h, which loads all of the needed widgets, and
//  web_init.h, which sets up the web page as needed.
//
//  The main function defined here is Live(), which is used to indicate that a variable
//  or function on a web page should be updated every time the page is redrawn, and not
//  just called once.
//

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

} // END web namespace
} // END emp namespace

#endif
