/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2017 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Listeners.hpp
 * @brief A class for tracking font event listeners for Widgets
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_LISTENERS_HPP_GUARD
#define INCLUDE_EMP_WEB_LISTENERS_HPP_GUARD



#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // #ifdef __EMSCRIPTEN__

#include <map>
#include <stddef.h>
#include <string>

#include "../tools/string_utils.hpp"

namespace emp::web {

  /// Track a set of JavaScript Listeners with their callback IDs.
  class Listeners {
  private:
    std::map<std::string, size_t> listeners;  ///< Map triggers to callback IDs

  public:
    Listeners() { ; }

    Listeners(const Listeners &)             = default;
    Listeners & operator=(const Listeners &) = default;

    /// How many listeners are we tracking?
    size_t GetSize() const { return listeners.size(); }

    /// Use a pre-calculated function ID with a new listener.
    Listeners & Set(const std::string & name, size_t fun_id) {
      emp_assert(!Has(name));
      listeners[name] = fun_id;
      return *this;
    }

    /// Calculate its own function ID with JSWrap.
    template <typename... Ts>
    Listeners & Set(const std::string & name, const std::function<void(Ts... args)> & in_fun) {
      emp_assert(!Has(name));
      listeners[name] = JSWrap(in_fun);
      return *this;
    }

    /// Determine if a specified listener exists.
    bool Has(const std::string & event_name) const {
      return listeners.find(event_name) != listeners.end();
    }

    /// Get the ID associated with a specific listener.
    size_t GetID(const std::string & event_name) {
      emp_assert(Has(event_name));
      return listeners[event_name];
    }

    const std::map<std::string, size_t> & GetMap() const { return listeners; }

    /// Remove all listeners
    void Clear() {
      // @CAO: Delete functions to be called.
      listeners.clear();
    }

    /// Remove a specific listener.
    void Remove(const std::string & event_name) {
      // @CAO: Delete function to be called.
      listeners.erase(event_name);
    }

    /// Apply all of the listeners being tracked.
    void Apply(const std::string & widget_id) {
      // Find the current object only once.
#ifdef __EMSCRIPTEN__
      MAIN_THREAD_EM_ASM(
        {
          var id        = UTF8ToString($0);
          emp_i.cur_obj = document.getElementById(id);
        },
        widget_id.c_str());
#endif  // #ifdef __EMSCRIPTEN__

      for (auto event_pair : listeners) {
#ifdef __EMSCRIPTEN__
        MAIN_THREAD_EM_ASM(
          {
            var name = UTF8ToString($0);
            if (emp_i.cur_obj) {
              emp_i.cur_obj.addEventListener(name, function(evt) { emp.Callback($1, evt); });
            }
          },
          event_pair.first.c_str(),
          event_pair.second);
#else   // #ifdef __EMSCRIPTEN__
        std::cout << "Setting '" << widget_id << "' listener '" << event_pair.first << "' to '"
                  << event_pair.second << "'.";
#endif  // #ifdef __EMSCRIPTEN__ : #else
      }
    }

    /// Apply a SPECIFIC listener.
    static void Apply(const std::string & widget_id, const std::string event_name, size_t fun_id) {
#ifdef __EMSCRIPTEN__
      MAIN_THREAD_EM_ASM(
        {
          var id      = UTF8ToString($0);
          var name    = UTF8ToString($1);
          var element = document.getElementById(id);
          if (element) {
            element.addEventListener(name, function(evt) { emp.Callback($2, evt); });
          }
        },
        widget_id.c_str(),
        event_name.c_str(),
        fun_id);
#else   // #ifdef __EMSCRIPTEN__
      std::cout << "Setting '" << widget_id << "' listener '" << event_name << "' to function id '"
                << fun_id << "'.";
#endif  // #ifdef __EMSCRIPTEN__ : #else
    }

    /// true/false : do any listeners exist?
    operator bool() const { return (bool) listeners.size(); }
  };


}  // namespace emp::web


#endif  // #ifndef INCLUDE_EMP_WEB_LISTENERS_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: evt
