/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2021-2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/notify.hpp
 * @brief Tools to alert users of messages (including errors and warnings) in a consistent manner.
 * @note Status: ALPHA
 *
 *
 * There are a handful of notification types to consider:
 * - Message: A simple notification.
 * - Verbose: Optional messages that can be activated by category.
 * - Warning: Something looks suspicious, but is not technically a problem (don't exit)
 * - Error: Something has gone horribly wrong and is impossible to recover from (exit)
 * - Exception: Something didn't go the way we expected, but we can still recover (exit if not handled)
 * - Debug: A simple notification that should only be printed when NDEBUG is not set (don't exit)
 *
 * These can be called as functions in the notify namespace with the appropriate name.
 *
 *   emp::notify::Message("The file '", filename, "' has successfully loaded.");
 *   emp::notify::Warning("Unused variable: ", var_name);
 *
 * Messages default to "standard out"; all of the other default to "standard error".  Handling of
 * these notifications can all be overridden by either whole category or by specific tag.
 *
 * All of these functions also have a Test* version where the first argument must be true for the
 * notification to occur.  For example:
 *
 *   emp::notify::TestError(x > 10, "The value x must be <= 10, but x = ", x);
 *
 * There are three possible recipients for all errors/warnings.
 * - The end-user if the problem stems from inputs they provided to the executable.
 * - The library user if the problem is due to mis-use of library functionality.
 * - The library developers if something that should be impossible occurs.
 *
 * These functions target the first group; developers should prefer asserts to ensure that
 * supposedly "impossible" situations do not occur.
 *
 * SHORTCUTS:
 * - emp::Alert(...) is a shortcut to emp::notify::Warning(...)
 *
 * NOTES:
 * - Whenever possible, Exception() should be preferred over Error().  They are can be responded
 *   to or selectively disabled, rather than always automatically halting execution.
 * - Warning() should ideally detail what should be done differently to avoid that warning.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_NOTIFY_HPP_GUARD
#define INCLUDE_EMP_BASE_NOTIFY_HPP_GUARD

#include <any>
#include <array>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace emp::notify {
  using id_t          = std::string;
  using message_t     = std::string;
  using except_data_t = std::any;

  using id_arg_t      = const id_t &;
  using message_arg_t = const message_t &;
  using response_t    = bool(id_arg_t, message_arg_t, except_data_t);
  using exit_fun_t    = std::function<void(int)>;

  /// Information about an exception that has occurred.
  struct ExceptInfo {
    id_t id           = "__NONE__";  ///< Which exception was triggered?
    message_t message = "";          ///< A detailed message of this exception.
    except_data_t data;              ///< Extra data needed to resolve this exception.
  };

  enum class Type { MESSAGE = 0, DEBUG, WARNING, ERROR, EXCEPTION, NUM_TYPES };
  static constexpr size_t num_types = static_cast<size_t>(Type::NUM_TYPES);

  /// Convert a type to a human-readable string.
  static id_t NotifyTypeID(Type type) {
    switch (type) {
      case Type::MESSAGE:   return "Message";
      case Type::DEBUG:     return "Debug";
      case Type::WARNING:   return "WARNING";
      case Type::ERROR:     return "ERROR";
      case Type::EXCEPTION: return "EXCEPTION";
      default:              return "Unknown";
    }
  }

  /// Convert a type to a human-readable string in COLOR.
  static id_t ColorTypeID(Type type) {
    const std::string green_text   = "\033[32m";
    const std::string magenta_text = "\033[35m";
    const std::string red_text     = "\033[31m";
    const std::string yellow_text  = "\033[33m";
    const std::string normal_text  = "\033[39m";
    const std::string bold_text    = "\033[1m";
    const std::string no_bold_text = "\033[22m";
    switch (type) {
      case Type::MESSAGE: return green_text + "Message" + normal_text;
      case Type::DEBUG:   return green_text + bold_text + "Debug" + no_bold_text + normal_text;
      case Type::WARNING: return yellow_text + bold_text + "WARNING" + no_bold_text + normal_text;
      case Type::ERROR:   return red_text + bold_text + "ERROR" + no_bold_text + normal_text;
      case Type::EXCEPTION:
        return magenta_text + bold_text + "EXCEPTION" + no_bold_text + normal_text;
      default: return "Unknown";
    }
  }

  // Maintain a specified collection of handlers.
  class HandlerSet {
  private:
    using fun_t          = std::function<response_t>;
    using fun_no_data_t  = std::function<bool(id_arg_t, message_arg_t)>;
    using fun_msg_only_t = std::function<bool(message_arg_t)>;
    std::vector<fun_t> handlers;
    bool exit_on_fail = false;

  public:
    bool GetExitOnFail() const { return exit_on_fail; }

    HandlerSet & SetExitOnFail(bool _exit = true) {
      exit_on_fail = _exit;
      return *this;
    }

    /// Trigger all handlers associated with a given ID.
    bool Trigger(id_arg_t id, message_arg_t message, except_data_t except_data) {
      // Run handlers from most recently added to oldest.
      for (auto it = handlers.rbegin(); it != handlers.rend(); ++it) {
        // Run until "true" result
        bool result = (*it)(id, message, except_data);
        if (result) {
          return true;  // Stop if any handler succeeded.
        }
      }

      return false;
    }

    // Trigger without providing data.
    bool Trigger(id_arg_t id, message_arg_t message) { return Trigger(id, message, 0); }

    // Trigger from a stored notification.
    bool Trigger(const ExceptInfo & info) { return Trigger(info.id, info.message, info.data); }

    // Add a function to this set.
    HandlerSet & Add(fun_t in) {
      handlers.push_back(in);
      return *this;
    }

    // Add a function with no data.
    HandlerSet & Add(fun_no_data_t in) {
      handlers.push_back(
        [fun = in](id_arg_t id, message_arg_t msg, except_data_t) { return fun(id, msg); });
      return *this;
    }

    // Add a function with only a single message
    HandlerSet & Add(fun_msg_only_t in) {
      handlers.push_back(
        [fun = in](id_arg_t, message_arg_t msg, except_data_t) { return fun(msg); });
      return *this;
    }

    // Clear all handlers associated with a given id.
    HandlerSet & Clear() {
      handlers.resize(0);
      return *this;
    }

    /// Replace all handlers with nothing (i.e., clear them)
    void Replace() { Clear(); }

    /// Replace all handlers with the generic ones provided.
    template <typename... FUN_Ts>
    void Replace(fun_t in, FUN_Ts... extra) {
      Replace(extra...);
      Add(in);
    }
  };

  /// Statically stored data about current notifications.
  struct NotifyData {
    // For each exception name we will keep a vector of handlers, appended to in the order
    // that they arrive (most recent will be last)
    std::unordered_map<id_t, HandlerSet> handler_map;   // Full set of notification handlers.
    std::unordered_map<std::string, bool> verbose_map;  // Set of categories for verbose messages.
    std::vector<exit_fun_t> exit_funs;                  // Set of handlers to run on exit.
    std::vector<ExceptInfo> except_queue;  // Unresolved exceptions after handlers have run
    std::vector<ExceptInfo> pause_queue;   // Unresolved notifications during pause
    bool lethal_exceptions = true;         // Should unresolved exceptions end the program?
    bool is_paused         = false;        // When paused, save notifications until unpaused.

    HandlerSet & GetHandler(Type type) { return handler_map[NotifyTypeID(type)]; }

    static void DefaultPrint(const std::string & msg, [[maybe_unused]] bool to_cerr = false) {
#if defined(__EMSCRIPTEN__)
      EM_ASM({
        msg = UTF8ToString($0);
        if (typeof alert == "undefined") { globalThis.alert = console.log; }
        alert(msg);
      }, msg.c_str());
#else   // #if defined(__EMSCRIPTEN__)
      if (to_cerr) {
        std::cerr << msg << std::endl;
      } else {
        std::cout << msg << std::endl;
      }
#endif  // #if defined(__EMSCRIPTEN__) : #else
    }

    NotifyData() {
      // Setup the default handlers and exit rules.
      GetHandler(Type::MESSAGE).Add([](id_arg_t, message_arg_t msg) {
        DefaultPrint(msg);
        return true;
      });

      // By default, debug printing occurs only in debug mode.
      GetHandler(Type::DEBUG)
        .Add(
#ifdef NDEBUG
          [](id_arg_t, message_arg_t) { return true; }
#else   // #ifdef NDEBUG
          [](id_arg_t, message_arg_t msg) {
            const std::string tag = ColorTypeID(Type::DEBUG);
            DefaultPrint(tag + ": " + msg);
            return true;
          }
#endif  // #ifdef NDEBUG : #else
        );

      GetHandler(Type::WARNING).Add([](id_arg_t, message_arg_t msg) {
        const std::string tag = ColorTypeID(Type::WARNING);
        DefaultPrint(tag + ": " + msg);
        return true;  // Only warning, do not exit.
      });

      GetHandler(Type::ERROR).Add([](id_arg_t, message_arg_t msg) {
        const std::string tag = ColorTypeID(Type::ERROR);
        DefaultPrint(tag + ": " + msg);
        return false;  // Does not correct the problem, so exit.
      });

      GetHandler(Type::EXCEPTION).Add([](id_arg_t id, message_arg_t msg) {
        const std::string tag = ColorTypeID(Type::EXCEPTION);
        std::stringstream ss;
        ss << tag << " (" << id << "): " << msg << std::endl;
        DefaultPrint(ss.str(), true);
        return false;  // Does not correct the problem, so exit.
      });
      GetHandler(Type::EXCEPTION).SetExitOnFail();

      // The initial exit handler should actually exit, using the appropriate exit code.
      exit_funs.push_back([](int code) { exit(code); });
    }
  };

  /// Central call to obtain NotifyData singleton.
  static NotifyData & GetData() {
    static NotifyData data;
    return data;
  }

  inline auto & MessageHandlers() { return GetData().GetHandler(Type::MESSAGE); }

  inline auto & DebugHandlers() { return GetData().GetHandler(Type::DEBUG); }

  inline auto & WarningHandlers() { return GetData().GetHandler(Type::WARNING); }

  inline auto & ErrorHandlers() { return GetData().GetHandler(Type::ERROR); }

  [[maybe_unused]] static void AddExitHandler(exit_fun_t fun) {
    GetData().exit_funs.push_back(fun);
  }

  [[maybe_unused]] static void ClearExitHandlers() { GetData().exit_funs.resize(0); }

  [[maybe_unused]] static void ReplaceExitHandlers() { ClearExitHandlers(); }

  template <typename... FUN_Ts>
  static void ReplaceExitHandlers(exit_fun_t fun, FUN_Ts... extras) {
    ReplaceExitHandlers(extras...);
    AddExitHandler(fun);
  }

  /// Generic exit handler that calls all of the provided functions.
  [[maybe_unused]] static void Exit(int exit_code) {
    NotifyData & data = GetData();

    // Run any cleanup functions.
    for (auto it = data.exit_funs.rbegin(); it != data.exit_funs.rend(); ++it) { (*it)(exit_code); }

    // Exit for real.
    exit(exit_code);
  }

  /// Generic Notification where type must be specified.
  template <typename... Ts>
  static bool Notify(Type type, Ts... args) {
    NotifyData & data = GetData();
    const id_t id     = NotifyTypeID(type);

    // Setup the message in a string stream.
    std::stringstream ss;
    ((ss << std::forward<Ts>(args)), ...);

    // If we are are paused, save this notification for later.
    if (data.is_paused) {
      data.pause_queue.push_back(ExceptInfo{id, ss.str(), 0});
      return true;
    }

    bool result = data.handler_map[id].Trigger(id, ss.str());

    // And return the success result.
    return result;
  }

  [[maybe_unused]] static void Pause() {
    NotifyData & data = GetData();
    data.is_paused    = true;
  }

  [[maybe_unused]] static void Unpause() {
    NotifyData & data = GetData();

    // Step through the notifications that have accrued.
    for (size_t i = 0; i < data.pause_queue.size(); ++i) {
      auto & notice = data.pause_queue[i];
      bool result   = data.handler_map[notice.id].Trigger(notice);
      if (!result) {  // Failed; move to exception queue or exit if error.
        if (notice.id == "ERROR") { Exit(1); }
        data.except_queue.push_back(notice);
      }
    }

    data.pause_queue.resize(0);  // Clear out the queue.

    data.is_paused = false;
  }

  /// Send out a regular notification.
  template <typename... Ts>
  static bool Message(Ts... args) {
    return Notify(Type::MESSAGE, std::forward<Ts>(args)...);
  }

  /// Send out a DEBUG notification.
  template <typename... Ts>
  static bool Debug(Ts... args) {
    return Notify(Type::DEBUG, std::forward<Ts>(args)...);
  }

  /// Send out a notification of a WARNING.
  template <typename... Ts>
  static bool Warning(Ts... args) {
    return Notify(Type::WARNING, std::forward<Ts>(args)...);
  }

  /// Send out a notification of an ERROR.
  template <typename... Ts>
  static bool Error(Ts... args) {
    bool success = Notify(Type::ERROR, std::forward<Ts>(args)...);
    if (!success) {
#ifdef NDEBUG
      Exit(1);
#else   // #ifdef NDEBUG
      abort();
#endif  // #ifdef NDEBUG : #else
    }
    return success;
  }

  // Print a message only if a specified condition is true.
  template <typename... Ts>
  static bool TestMessage(bool test, Ts... args) {
    if (test) { return Message(std::forward<Ts>(args)...); }
    return true;
  }

  // Trigger a debug output only if a specified condition is true.
  template <typename... Ts>
  static bool TestDebug(bool test, Ts... args) {
    if (test) { return Debug(std::forward<Ts>(args)...); }
    return true;
  }

  // Trigger a warning only if a specified condition is true.
  template <typename... Ts>
  static bool TestWarning(bool test, Ts... args) {
    if (test) { return Warning(std::forward<Ts>(args)...); }
    return true;
  }

  // Trigger an error only if a specified condition is true.
  template <typename... Ts>
  static bool TestError(bool test, Ts... args) {
    if (test) { return Error(std::forward<Ts>(args)...); }
    return true;
  }

  /// Add a handler for a particular exception type.
  template <typename FUN_T>
  static HandlerSet & AddHandler(id_arg_t id, FUN_T fun) {
    return GetData().handler_map[id].Add(fun);
  }

  /// Add a generic exception handler.
  template <typename FUN_T>
  static HandlerSet & AddHandler(FUN_T fun) {
    return GetData().handler_map["EXCEPTION"].Add(fun);
  }

  /// Ignore exceptions of a specific type.
  [[maybe_unused]] static HandlerSet & Ignore(id_arg_t id) {
    return AddHandler(id, [](id_arg_t, message_arg_t) { return true; });
  }

  /// Turn on a particular verbosity category.
  [[maybe_unused]] static void SetVerbose(std::string id, bool make_active = true) {
    GetData().verbose_map[id] = make_active;
  }

  /// Send out a notification of an "verbose" message.
  template <typename... Ts>
  [[maybe_unused]] static bool Verbose(const std::string & id, Ts... args) {
    NotifyData & data = GetData();

    if (data.verbose_map[id]) { return Notify(Type::MESSAGE, std::forward<Ts>(args)...); }

    return false;
  }

  /// Send out a notification of an Exception.
  [[maybe_unused]] static bool Exception(id_arg_t id,
                                         message_arg_t message     = "",
                                         except_data_t except_data = 0) {
    NotifyData & data = GetData();

    if (data.is_paused) {
      data.pause_queue.push_back(ExceptInfo{id, message, except_data});
      return true;
    }

    // Retrieve any specialized exception handlers for this type of exception.
    bool result = data.handler_map[id].Trigger(id, message, except_data);

    // If unresolved, see if we should quit; else use a generic exception handler.
    if (!result) {
      if (data.handler_map[id].GetExitOnFail()) { Exit(1); }
      result = data.handler_map["EXCEPTION"].Trigger(id, message, except_data);
    }

    // If still unresolved, either give up or save the exception for later analysis.
    if (!result) {
      if (data.handler_map["EXCEPTION"].GetExitOnFail()) { Exit(1); }
      data.except_queue.push_back(ExceptInfo{id, message, except_data});
    }

    return result;
  }

  // Trigger a debug output only if a specified condition is true.
  template <typename... Ts>
  static bool TestException(bool test, Ts... args) {
    if (test) { return Exception(std::forward<Ts>(args)...); }
    return true;
  }

  /// Retrieve a vector of ALL unresolved exceptions.
  [[maybe_unused]] static const std::vector<ExceptInfo> & GetExceptions() {
    return GetData().except_queue;
  }

  /// Retrieve the first unresolved exception with a given id.
  [[maybe_unused]] static ExceptInfo GetException(id_arg_t id) {
    for (ExceptInfo & x : GetData().except_queue) {
      if (x.id == id) { return x; }
    }
    return ExceptInfo{};
  }

  /// Return a total count of how many unresolved exceptions are left.
  [[maybe_unused]] static size_t CountExceptions() { return GetData().except_queue.size(); }

  /// Return a total count of how many unresolved exceptions have a given id.
  [[maybe_unused]] static size_t CountExceptions(id_arg_t id) {
    size_t count = 0;
    for (ExceptInfo & x : GetData().except_queue) {
      if (x.id == id) { ++count; }
    }
    return count;
  }

  /// Identify whether there are ANY unresolved exceptions.
  [[maybe_unused]] static bool HasExceptions() { return CountExceptions(); }

  /// Identify whether there are any unresolved exceptions with a given id.
  [[maybe_unused]] static bool HasException(id_arg_t id) {
    for (ExceptInfo & x : GetData().except_queue) {
      if (x.id == id) { return true; }
    }
    return false;
  }

  /// Remove all unresolved exceptions.
  [[maybe_unused]] static void ClearExceptions() { GetData().except_queue.resize(0); }

  /// Remove first exception with a given id.
  [[maybe_unused]] static void ClearException(id_arg_t id) {
    auto & except_queue = GetData().except_queue;
    for (size_t i = 0; i < except_queue.size(); ++i) {
      if (except_queue[i].id == id) {
        // If exception is NOT in the last position, move last position earlier and reduce size.
        if (i < except_queue.size() - 1) { except_queue[i] = except_queue.back(); }
        except_queue.resize(except_queue.size() - 1);
        return;
      }
    }
  }

}  // namespace emp::notify

// Shortcuts in emp for notify functions.
namespace emp {
  template <typename... Ts>
  static bool Alert(Ts... args) {
    return notify::Warning(std::forward<Ts>(args)...);
  }

  /// A version of Alert that will cap how many times it can go off
  template <typename... Ts>
  static bool CappedAlert(size_t cap, Ts... inputs) {
    static size_t cur_count = 0;
    if (++cur_count <= cap) { return Alert(std::forward<Ts>(inputs)...); }
    return false;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_NOTIFY_HPP_GUARD
