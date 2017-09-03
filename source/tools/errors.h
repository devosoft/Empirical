//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  These tools help manage various problems for command-line or Emscripten-based applications.
//  Status: ALPHA
//
//
//  There are three possible recipiants for all errors/warnings.
//  * The end-user if the problem stems from inputs they provided to the executable.
//  * The library user if the problem is due to mis-use of library functionality.
//  * The library developers if something that should be impossible occurs.
//
//  There are also three types of problmes to notify about:
//  * Warnings if something looks suspicious, but isn't technically a problem.
//  * Errors if something has gone so horribly wrong that it is impossible to recover from.
//  * Exceptions if something didn't go the way we expected, but we can still recover.
//
//  NOTES:
//  * Whenever possible, exceptions should be preferred.  They are more specific than warnings,
//    but don't halt execution like errors.
//  * Asserts should usually be used instead of Errors when the target audience is a developer.
//    (A user won't know what to do with a failed assert, while an error can abort gracefully.)
//  * Warnings should always specify what should be done differently to surpress the warning.
//
//
//  Development Notes:
//  * We should move over to a pure replacement for exceptions.
//    - Different types of exceptions can trigger a signal.  Actions should return a bool
//      indicating whether the exception was fixed.
//    - Remaining exceptions are recorded and passed back up the chain to (hopefully) be caught.
//    - Uncaught exceptions should have a default behavior when Resolved.  Exceptions could have
//      various resolve times: Next exception added, Next exception check, when ResolveExceptions()
//      is run, End of program, or ASAP. (perhaps)

#ifndef EMP_ERRORS_H
#define EMP_ERRORS_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "../meta/meta.h"

namespace emp {

  struct ExceptInfo {
    std::string id;
    std::string desc;
    bool default_to_error;  // Should we default to an error (or a warning) if not resolved?
  };

  static const ExceptInfo & GetEmptyExcept() {
    static ExceptInfo fail_info{"","",false};
    return fail_info;
  }

  static std::multimap<std::string, ExceptInfo> & GetExceptMap() {
    static std::multimap<std::string, ExceptInfo> except_map;
    return except_map;
  }

  inline void TriggerExcept(const std::string & in_id, const std::string & in_desc, bool in_error=true) {
    GetExceptMap().emplace(in_id, ExceptInfo({in_id, in_desc, in_error}));
  }

  inline const ExceptInfo & GetExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    if (it != fail_map.end()) return it->second;
    return GetEmptyExcept();
  }

  inline ExceptInfo PopExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    auto out = GetEmptyExcept();
    if (it != fail_map.end()) { out = it->second; fail_map.erase(it); }
    return out;
  }

  inline size_t CountExcepts() { return GetExceptMap().size(); }
  inline bool HasExcept() { return CountExcepts(); }
  inline bool HasExcept(const std::string & id) { return GetExceptMap().count(id); }
  inline void ClearExcepts() { GetExceptMap().clear(); }
  inline void ClearExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    if (it != fail_map.end()) fail_map.erase(it);
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

}


#endif
