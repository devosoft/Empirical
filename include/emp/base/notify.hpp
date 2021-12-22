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

#include <any>
#include <array>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "vector.hpp"

namespace emp {
namespace notify {
  using response_t = bool(const std::string & /*id*/, const std::string & /*message*/);
  using no_id_response_t = bool(const std::string & /*message*/);
  using exit_fun_t = void(size_t);

  /// Information about an exception that has occurred.
  struct ExceptInfo {
    std::string id = "__NONE__";  ///< Which exception was triggered?
    std::string message = "";     ///< A detailed message of this exception.
    std::any data;                ///< Extra data needed to resolve this exception.
  };

  enum class Type { MESSAGE=0, DEBUG, WARNING, ERROR, EXCEPTION, NUM_TYPES };
  static constexpr size_t num_types = static_cast<size_t>(Type::NUM_TYPES);

  /// Convert a type to a human-readable string.
  static std::string TypeName(Type type) {
    switch (type) {
      case Type::MESSAGE: return "Message";
      case Type::DEBUG: return "Debug";
      case Type::WARNING: return "WARNING";
      case Type::ERROR: return "ERROR";
      case Type::EXCEPTION: return "EXCEPTION";
      default: return "Unknown";
    }
  }

  // Maintain a specified collection of handlers.
  class HandlerSet {
  private:
    using fun_t = std::function<response_t>;
    emp::vector<fun_t> handlers;

  public:
    HandlerSet() {}
    HandlerSet(const HandlerSet &) = default;
    HandlerSet(HandlerSet &&) = default;
    ~HandlerSet() { }

    /// Trigger all handlers associated with a given ID.
    bool Trigger(const std::string & id, const std::string & message, std::any data) {
      // Run handlers from most recently added to oldest.
      for (auto it = handlers.rbegin();
            it != handlers.rend();
            ++it) {
        // Run until "true" result
        bool result = (*it)(id, message);
        if (result) return true;           // Stop if any handler succeeded.
      }

      return false;
    }

    // Trigger without providing data.
    bool Trigger(const std::string & id, const std::string & message)  {
      return Trigger(id, message, 0);
    }

    // Add a function to this set.
    HandlerSet & Add(fun_t in) { handlers.push_back(in); return *this; }

    // Clear all handlers associated with a given id.
    HandlerSet & Clear() { handlers.resize(0); return *this; }
 
    /// Replace all handlers with nothing (i.e., clear them)
    void Replace() { Clear(); }

    /// Replace all handelers with the generic ones provided.
    template <typename... FUN_Ts>
    void Replace(fun_t in, FUN_Ts... extra) {
      Replace(extra...);
      Add(in);
    }
  };

  /// Staticly stored data about current notifications.
  struct NotifyData {
    // For each exception name we will keep a vector of handlers, appended to in the order
    // that they arrive (most recent will be last)
    std::unordered_map<std::string, HandlerSet> handler_map;
    emp::vector<std::function<exit_fun_t>> exit_funs;        // Set of handlers to run on exit.
    emp::vector<ExceptInfo> except_queue;                    // Unresolved exceptions

    HandlerSet & GetHandler(Type type) { return handler_map[TypeName(type)]; }

    NotifyData() {
      // Setup the default handlers and exit rules.
      GetHandler(Type::MESSAGE).Add(
        [](const std::string & /*id*/, const std::string & msg) {
          std::cout << msg << std::endl;
          return true;
        }
      );

      GetHandler(Type::DEBUG).Add(
#ifdef NDEBUG
        [](const std::string & /*id*/, const std::string & msg){ return true; }
#else
        [](const std::string & /*id*/, const std::string & msg) {
          std::cout << "Debug: " << msg << std::endl;
          return true;
        }
#endif
      );

      GetHandler(Type::WARNING).Add(
        [](const std::string & /*id*/, const std::string & msg) {
          std::cerr << "WARNING: " << msg << std::endl;
          return true;
        }
      );

      GetHandler(Type::ERROR).Add(
        [](const std::string & /*id*/, const std::string & msg) {
          std::cerr << "ERROR: " << msg << std::endl;
          return true;
        }
      );

      GetHandler(Type::EXCEPTION).Add(
        [](const std::string & id, const std::string & msg) {
          std::cerr << "EXCEPTION (" << id << "): " << msg << std::endl;
          return false;
        }
      );

      // The initial exit handler should actually exit, using the appropriate exit code.
      exit_funs.push_back( [](size_t code){ exit(code); } );
    }
  };

  /// Central call to obtain NotifyData singleton.
  static NotifyData & GetData() { static NotifyData data; return data; }
  auto & MessageHandlers() { return GetData().GetHandler(Type::MESSAGE); }
  auto & DebugHandlers() { return GetData().GetHandler(Type::DEBUG); }
  auto & WarningHandlers() { return GetData().GetHandler(Type::WARNING); }
  auto & ErrorHandlers() { return GetData().GetHandler(Type::ERROR); }
  auto & ExceptionHandlers() { return GetData().GetHandler(Type::EXCEPTION); }
  auto & ExitHandlers() { return GetData().exit_funs; }

  /// Generic exit handler that calls all of the provided functions.
  static void Exit(size_t exit_code) {
    NotifyData & data = GetData();
    for (auto it = data.exit_funs.rbegin(); it != data.exit_funs.rend(); ++it) {
      (*it)(exit_code);
    }
  }

  /// Generic Notification where type must be specified.
  template <typename... Ts>
  static bool Notify(Type type, Ts... args) {
    NotifyData & data = GetData();
    const std::string id = TypeName(type);

    // Setup the message in a string stream.
    std::stringstream ss;
    ((ss << std::forward<Ts>(args)), ...);

    // Run the appropriate handler
    bool result = data.handler_map[id].Trigger(id, ss.str());

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
  static bool Error(Ts... args) {
    bool success = Notify(Type::ERROR, std::forward<Ts>(args)...);
    if (!success) Exit(1);
    return success;
  }


  /// Send out a notification of an Exception.
  template <typename... Ts>
  static bool Exception(const std::string & id, Ts... args) { 
    NotifyData & data = GetData();

    // Setup the message in a string stream.
    std::stringstream ss;
    ((ss << std::forward<Ts>(args)), ...);

    // Retrieve the exception handlers that we have for this type of exception.
    bool result = ExceptionHandlers().Trigger(id, ss.str());

    // If unresolved, either give up or save the exception for later analysis.
    if (!result) data.except_queue.push_back(ExceptInfo{id, ss.str()});

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

}
}


#endif