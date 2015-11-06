// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_ERRORS_H
#define EMP_ERRORS_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  These tools help manage error handling for command-line or Emscripten-based applications.
//

#include <iostream>
#include <string>

namespace emp {

  void Notify(std::string msg) {
#ifdef EMSCRIPTEN
    EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str());
#else
    std::cerr << msg << std::endl;
#endif
  }

  void NotifyWarning(std::string msg) {
    std::string print_msg = "WARNING: ";
    print_msg += msg;
    Notify(print_msg);
  }

  void NotifyError(std::string msg) {
    std::string print_msg = "ERROR: ";
    print_msg += msg;
    Notify(print_msg);
  }

};


#endif

