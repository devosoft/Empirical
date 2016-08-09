//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  These tools help manage error handling for command-line or Emscripten-based applications.
//
//  There are three possible recipiants for all errors.
//  * The end-user if the problem stems from inputs they provided to the executable.
//  * The library user if the problem is due to mis-use of library functionality.
//  * The library developers if something that should be impossible occurs.
//
//  There are also three types of problmes to notify about:
//  * Warnings if something looks suspicious, but isn't technically a problem.
//  * Errors if something has gone so horribly wrong that it is impossible to recover from.
//  * Failures if something didn't go the way we expected, but we can still recover.
//
//  Whenever possible, failues should be preferred.  They are more specific than warnings, but
//  don't halt execution like errors.

#ifndef EMP_ERRORS_H
#define EMP_ERRORS_H

#include <iostream>
#include <sstream>
#include <string>

#include "meta.h"
#include "vector.h"

namespace emp {

  struct FailueInfo {
    std::string id;
    std::string desc;
    bool default_to_error;  // Should we default to an error (or a warning) if not resolved?
  };

  static emp::vector<FailureInfo> & GetFailureList {
    static emp::vector<FailureInfo> failure_list;
    return failure_list;
  }

  template <typename... Ts>
  void Notify(Ts... args) {
    std::stringstream ss;
    EMP_EXPAND_PPACK( (ss << args) );
#ifdef EMSCRIPTEN
    EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, ss.str().c_str());
#else
    std::cerr << ss.str() << std::endl;
#endif
  }

  // End user has done something possibly a problem.
  template <typename... Ts>
  void NotifyWarning(Ts... msg) { Notify("WARNING: ", msg...); }

  // End user has done something resulting in an non-recoverable problem.
  template <typename... Ts>
  void NotifyError(Ts... msg) { Notify("ERROR: ", msg...); }

  // Library user has made an error in how they are using the library.
  template <typename... Ts>
  void LibraryWarning(Ts... msg) { Notify("EMPIRICAL USE WARNING: ", msg...); }

  // Library user has made an error in how they are using the library.
  template <typename... Ts>
  void LibraryError(Ts... msg) { Notify("EMPIRICAL USE ERROR: ", msg...); }

  // Original library implementers must have made an error.
  template <typename... Ts>
  void InternalError(Ts... msg) { Notify("INTERNAL EMPIRICAL ERROR: ", msg...); }

};


#endif
