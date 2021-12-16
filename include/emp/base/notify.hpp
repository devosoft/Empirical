/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  notify.hpp
 *  @brief Tools to alert users of messages (including errors and warnings) in a consistant manner.
 *  @note Status: ALPHA
 *
 *
 *  There are five types of notifications to consider:
 *  - Message: A simple notification.
 *  - Warning: Something looks suspicious, but is not technically a problem (don't exit)
 *  - Error: Something has gone horribly wrong and is impossible to recover from (exit)
 *  - Exception: Something didn't go the way we expected, but we can still recover (exit if not handled)
 *  - Debug: A simple notification that should only be printed when NDEBUG is not set (don't exit)
 *
 *  Messages default to "standard out"; all of the other default to "standard error".  Handling of
 *  these notifications can all be overriden by either whole category or by specific tag.
 * 
 *  There are three possible recipients for all errors/warnings.
 *  - The end-user if the problem stems from inputs they provided to the executable.
 *  - The library user if the problem is due to mis-use of library functionality.
 *  - The library developers if something that should be impossible occurs.
 * 
 *  The content of this file primarily targets the first group; developers should prefer asserts
 *  to ensure that supposedly "impossible" situations do not occur.
 *
 *  NOTES:
 *  - Whenever possible, exceptions should be preferred.  They are more specific than warnings
 *    and can be responded to rather than automatically halting execution like errors.
 *  - Warnings should always detail what should be done differently to surpress that warning.
 *
 */

#ifndef EMP_NOTIFY_HPP
#define EMP_NOTIFY_HPP

#include <array>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "vector.hpp"

namespace emp {
namespace notify {
  using response_t = std::function<bool(const std::string & /*id*/, const std::string & /*desc*/)>;
  using no_id_response_t = std::function<bool(const std::string & /*desc*/)>;
  using response_vec_t = emp::vector<response_t>;
  using exit_handler_t = std::function<void(size_t)>;

  /// Information about an exception that has occurred.
  struct ExceptInfo {
    std::string id = "__NONE__";  ///< A unique string ID for this exception type.
    std::string desc = "";        ///< A detailed description of this exception.
  };

  enum class Type { MESSAGE=0, DEBUG, WARNING, ERROR, EXCEPTION, NUM_TYPES };

  std::string TypeName(Type type) {
    switch (type) {
      case Type::MESSAGE: return "Message";
      case Type::DEBUG: return "Debug";
      case Type::WARNING: return "WARNING";
      case Type::ERROR: return "ERROR";
      case Type::EXCEPTION: return "EXCEPTION";
      default: return "Unknown";
    }
  }

  /// Staticly stored data about current notifications.
  struct NotifyData {
    // For each exception name we will keep a vector of handlers, appended to in the order
    // that they arrive (most recent will be last)
    std::unordered_map<std::string, response_vec_t> except_handlers;

    emp::vector<ExceptInfo> except_queue;                         // Unresolved exceptions
    std::array<response_t, (size_t) Type::NUM_TYPES> handlers;    // Default handlers for notifications
    std::array<bool, (size_t) Type::NUM_TYPES> exit_on;           // Should we exit on given msg type?

    emp::vector<exit_handler_t> exit_handlers;                    // Set of handlers to run on exit.

    NotifyData() {
      // Setup the default handlers and exit rules.
      handlers[(size_t) Type::MESSAGE] = 
        [](const std::string & /*id*/, const std::string & msg) {
          std::cout << msg << std::endl;
          return true;
        };
      exit_on[(size_t) Type::MESSAGE] = false;

      handlers[(size_t) Type::DEBUG] = 
#ifdef NDEBUG
        [](const std::string & /*id*/, const std::string & msg){ return true; };
#else
        [](const std::string & /*id*/, const std::string & msg) {
          std::cout << "Debug: " << msg << std::endl;
          return true;
        };
#endif
      exit_on[(size_t) Type::DEBUG] = false;

      handlers[(size_t) Type::WARNING] = 
        [](const std::string & /*id*/, const std::string & msg) {
          std::cerr << "WARNING: " << msg << std::endl;
          return true;
        };
      exit_on[(size_t) Type::WARNING] = false;

      handlers[(size_t) Type::ERROR] = 
        [](const std::string & /*id*/, const std::string & msg) {
          std::cerr << "ERROR: " << msg << std::endl;
          return true;
        };
      exit_on[(size_t) Type::ERROR] = true;

      handlers[(size_t) Type::EXCEPTION] = 
        [](const std::string & id, const std::string & msg) {
          std::cerr << "EXCEPTION (" << id << "): " << msg << std::endl;
          return false;
        };
      exit_on[(size_t) Type::EXCEPTION] = true;  // When unhandled...

      // The initial exit handler should actually exit, using the appropriate exit code.
      exit_handlers.push_back( [](size_t code){ exit(code); } );
    }
  };

  /// Central call to obtain NotifyData singleton.
  static NotifyData & GetData() { static NotifyData data; return data; }

  /// Generic exit handler that calls all of the provided functions.
  static void Exit(size_t exit_code) {
    NotifyData & data = GetData();

    // Run exit handlers from most recently added to oldest.
    for (auto it = data.exit_handlers.rbegin();
          it != data.exit_handlers.rend();
          ++it) {
      (*it)(1);
    }
  }

  /// Generic Notification where type must be specified.
  template <typename... Ts>
  static bool Notify(Type type, Ts... args) {
    NotifyData & data = GetData();
    size_t type_id = (size_t) type;

    // Setup the message in a string stream.
    std::stringstream ss;
    ((ss << std::forward<Ts>(args)), ...);

    // Run the appropriate handler
    bool result = data.handlers[type_id](TypeName(type), ss.str());

    // Test if we are supposed to exit.
    if (data.exit_on[type_id]) Exit(1);

    // And return the success result.
    return result;
  }

  /// Send out a regular notification.
  template <typename... Ts>
  static bool Message(Ts... args) { return Notify(Type::MESSAGE, std::forward<Ts>(args)...); }

  /// Send out a DEBUG notification.
  template <typename... Ts>
  static bool Debug(Ts... args) { return Notify(Type::DEBUG, std::forward<Ts>(args)...); }

  /// Send out a notification of a WARNING.
  template <typename... Ts>
  static bool Warning(Ts... args) { return Notify(Type::WARNING, std::forward<Ts>(args)...); }

  /// Send out a notification of an ERROR.
  template <typename... Ts>
  static bool Error(Ts... args) { return Notify(Type::ERROR, std::forward<Ts>(args)...); }


  /// Send out a notification of an Exception.
  template <typename... Ts>
  static bool Exception(const std::string & id, Ts... args) { 
    NotifyData & data = GetData();

    // Setup the message in a string stream.
    std::stringstream ss;
    ((ss << std::forward<Ts>(args)), ...);

    bool result = false;

    // Retrieve the exception handlers that we have for this type of exception.
    auto & handlers = data.except_handlers[id];

    // See if any of them resolve they problem (trying the most recent first).
    for (auto it = handlers.rbegin(); it != handlers.rend() && result == false; ++it) {
      result |= (*it)(id, ss.str());
    }

    // If it's unresolved, try the default handler
    if (!result) {
      result = data.handlers[(size_t) Type::EXCEPTION](id, ss.str());
    }

    // If still unresolved, either give up or save the exception for later analysis.
    if (!result) {
      if (data.exit_on[(size_t) Type::EXCEPTION]) Exit(1);
      data.except_queue.push_back(ExceptInfo{id, ss.str()});
    }

    return result;
  }

  /// Retrieve a vector of ALL unresolved exceptions.
  static emp::vector<ExceptInfo> & GetExceptions() { return GetData().except_queue; }

  /// Retrieve the first unresolved exception with a given id.
  static ExceptInfo GetException(const std::string & id) {
    for (ExceptInfo & x : GetData().except_queue) if (x.id == id) return x;
    return ExceptInfo{};
  }

  /// Return a total count of how many unresolved exceptions are left.
  static size_t CountExceptions() { return GetData().except_queue.size(); }

  /// Return a total count of how many unresolved exceptions have a given id.
  static size_t CountExceptions(const std::string & id) {
    size_t count = 0;
    for (ExceptInfo & x : GetData().except_queue) if (x.id == id) ++count;
    return count;
  }

  /// Identify whether there are ANY unresolved exceptions.
  static bool HasExceptions() { return CountExceptions(); }

  /// Identify whether there are any unresolved exceptions with a given id.
  static bool HasException(const std::string & id) {
    for (ExceptInfo & x : GetData().except_queue) if (x.id == id) return true;
    return false;
  }

  /// Remove all unresolved exceptions.
  static void ClearExceptions() { GetData().except_queue.resize(0); }

  /// Remove first exception with a given id.
  static void ClearException(const std::string & id) {
    auto & except_queue = GetData().except_queue;
    for (size_t i = 0; i < except_queue.size(); ++i) {
      if (except_queue[i].id == id) {
        // If exception is NOT in the last position, move last position earlier and reduce size.
        if (i < except_queue.size() - 1) except_queue[i] = except_queue.back();
        except_queue.resize(except_queue.size() - 1);
        return;
      }
    }
  }

  template <typename FUN_T>
  static void AddExitHandler(FUN_T in) { GetData().exit_handlers.push_back(in); }
  static void ClearExitHandlers() { GetData().exit_handlers.resize(0); }

  static void ReplaceExitHandlers() { ClearExitHandlers(); }

  template <typename FUN1_T, typename... FUN_Ts>
  static void ReplaceExitHandlers(FUN1_T in, FUN_Ts... extra) {
    ReplaceExitHandlers(extra...);
    AddExitHandler(in);
  }

  static void SetExitOnMessage(bool in=true) { GetData().exit_on[(size_t) Type::MESSAGE] = in; }
  static void SetExitOnDebug(bool in=true) { GetData().exit_on[(size_t) Type::DEBUG] = in; }
  static void SetExitOnWarning(bool in=true) { GetData().exit_on[(size_t) Type::WARNING] = in; }
  static void SetExitOnError(bool in=true) { GetData().exit_on[(size_t) Type::ERROR] = in; }
  static void SetExitOnException(bool in=true) { GetData().exit_on[(size_t) Type::EXCEPTION] = in; }

  static void SetMessageHandler(response_t in) { GetData().handlers[(size_t) Type::MESSAGE] = in; }
  static void SetDebugHandler(response_t in) { GetData().handlers[(size_t) Type::DEBUG] = in; }
  static void SetWarningHandler(response_t in) { GetData().handlers[(size_t) Type::WARNING] = in; }
  static void SetErrorHandler(response_t in) { GetData().handlers[(size_t) Type::ERROR] = in; }
  static void SetExceptionHandler(response_t in) { GetData().handlers[(size_t) Type::EXCEPTION] = in; }
  static void AddExceptionHandler(const std::string & id, response_t in) {
    GetData().except_handlers[id].push_back(in);
  }
  static void ClearExceptionHandler(const std::string & id) {
    GetData().except_handlers.erase(id);
  }

  // Allow handlers that don't use the ID.
  static void SetMessageHandler(no_id_response_t in) {
    SetMessageHandler( [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }
  static void SetDebugHandler(no_id_response_t in) {
    SetDebugHandler( [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }
  static void SetWarningHandler(no_id_response_t in) {
    SetWarningHandler( [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }
  static void SetErrorHandler(no_id_response_t in) {
    SetErrorHandler( [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }
  static void SetExceptionHandler(no_id_response_t in) {
    SetExceptionHandler( [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }
  static void AddExceptionHandler(const std::string & id, no_id_response_t in) {
    AddExceptionHandler( id, [fun=in](const std::string &, const std::string & msg){ return fun(msg); } );
  }

}
}


#endif
