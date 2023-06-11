/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file errors.hpp
 *  @brief Tools to help manage various problems in command-line or Emscripten-based applications.
 *  @note Status: ALPHA
 *
 *  There are three possible recipiants for all errors/warnings.
 *  - The end-user if the problem stems from inputs they provided to the executable.
 *  - The library user if the problem is due to mis-use of library functionality.
 *  - The library developers if something that should be impossible occurs.
 *
 *  There are also three types of problmes to notify about:
 *  - Warnings if something looks suspicious, but isn't technically a problem.
 *  - Errors if something has gone so horribly wrong that it is impossible to recover from.
 *  - Exceptions if something didn't go the way we expected, but we can still recover.
 *
 *  In general, most of the content of this file is targeted at providing useful tools for library
 *  users; end-users should receive more customized messages and asserts should capture
 *  suposedly "impossible" situations that none-the-less occur in the library itself.
 *
 *  NOTES:
 *  - Whenever possible, exceptions should be preferred.  They are more specific than warnings,
 *    but don't halt execution like errors.
 *  - Warnings should always detail what should be done differently to surpress the warning.
 *
 *
 *  @todo We should move over to a pure replacement for exceptions.
 *    - Different types of exceptions can trigger a signal.  Actions should return a bool
 *      indicating whether the exception was fixed.
 *    - Remaining exceptions are recorded and passed back up the chain to (hopefully) be caught.
 *    - Uncaught exceptions should have a default behavior when Resolved.  Exceptions could have
 *      various resolve times: Next exception added, Next exception check, when ResolveExceptions()
 *      is run, End of program, or ASAP. (perhaps)
 */

#ifndef EMP_BASE_ERRORS_HPP_INCLUDE
#define EMP_BASE_ERRORS_HPP_INCLUDE

#include <iostream>
#include <map>
#include <sstream>
#include <string>

/// If we are in emscripten, make sure to include the header.
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace emp {

  /// Information about an exception that has occured.
  struct ExceptInfo {
    std::string id;         ///< A unique string ID for this exception type.
    std::string desc;       ///< A detailed description of thie exception.
    bool default_to_error;  ///< Should we default to an error (or a warning) if not resolved?
  };

  /// Function to generate an empty exception (returned when an exception is checked, but none exist.)
  static const ExceptInfo & GetEmptyExcept() {
    static ExceptInfo fail_info{"","",false};
    return fail_info;
  }

  /// A map of all exceptions that have occurred and are awaiting to be dealt with.
  static std::multimap<std::string, ExceptInfo> & GetExceptMap() {
    static std::multimap<std::string, ExceptInfo> except_map;
    return except_map;
  }

  /// Provide information about an exception that needs to be triggered.
  inline void TriggerExcept(const std::string & in_id, const std::string & in_desc, bool in_error=true) {
    GetExceptMap().emplace(in_id, ExceptInfo({in_id, in_desc, in_error}));
  }

  /// Get the first waiting exception.
  inline const ExceptInfo & GetExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    if (it != fail_map.end()) return it->second;
    return GetEmptyExcept();
  }

  /// Get and *remove* a waiting exception.
  inline ExceptInfo PopExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    auto out = GetEmptyExcept();
    if (it != fail_map.end()) { out = it->second; fail_map.erase(it); }
    return out;
  }

  /// How many exceptions are waiting to be dealt with?
  inline size_t CountExcepts() { return GetExceptMap().size(); }

  /// Are any exceptions waiting?
  inline bool HasExcept() { return CountExcepts(); }

  /// Are any exceptions of a particular type waiting?
  inline bool HasExcept(const std::string & id) { return GetExceptMap().count(id); }

  /// Remove all waiting exceptions.
  inline void ClearExcepts() { GetExceptMap().clear(); }

  /// Remove all waiting exceptions of the desginated type.
  inline void ClearExcept(const std::string & id) {
    auto & fail_map = GetExceptMap();
    auto it = fail_map.find(id);
    if (it != fail_map.end()) fail_map.erase(it);
  }

  namespace {
    // Copy all of the args into the stringstream.
    // Base case
    void Notify_impl(std::stringstream &) { ; }

    // For each arg, copy it into the provided stringstream and recurse to do the rest.
    template <typename T, typename... Ts>
    void Notify_impl(std::stringstream & ss, T && arg1, Ts &&... args) {
      ss << std::forward<T>(arg1);
      Notify_impl(ss, std::forward<Ts>(args)...);
    }
  }

  /// Send information to a program user (via standard error in native mode, or alert in Emscripten)
  template <typename... Ts>
  void Notify(Ts &&... args) {
    std::stringstream ss;
    Notify_impl(ss, std::forward<Ts>(args)...);
#ifdef __EMSCRIPTEN__
    EM_ASM_ARGS({
      msg = UTF8ToString($0);
      if (typeof alert == "undefined") {
        // node polyfill
        globalThis.alert = console.log;
      }
      alert(msg);
    }, ss.str().c_str());
#else
    std::cerr << ss.str() << std::endl;
#endif
  }

  /// End user has done something possibly a problem.
  template <typename... Ts>
  void NotifyWarning(Ts &&... msg) { Notify("WARNING: ", std::forward<Ts>(msg)...); }

  /// End user has done something resulting in an non-recoverable problem.
  template <typename... Ts>
  void NotifyError(Ts &&... msg) { Notify("ERROR: ", std::forward<Ts>(msg)...); }

  /// Library user has made an error in how they are using the library.
  template <typename... Ts>
  void LibraryWarning(Ts &&... msg) { Notify("EMPIRICAL USE WARNING: ", std::forward<Ts>(msg)...); }

  /// Library user has made an error in how they are using the library.
  template <typename... Ts>
  void LibraryError(Ts &&... msg) { Notify("EMPIRICAL USE ERROR: ", std::forward<Ts>(msg)...); }

  /// Original library implementers must have made an error.
  template <typename... Ts>
  void InternalError(Ts &&... msg) { Notify("INTERNAL EMPIRICAL ERROR: ", std::forward<Ts>(msg)...); }

}


#endif // #ifndef EMP_BASE_ERRORS_HPP_INCLUDE
