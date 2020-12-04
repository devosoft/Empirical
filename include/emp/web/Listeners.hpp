/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  Listeners.hpp
 *  @brief A class for tracking font event listeners for Widgets
 */


#ifndef EMP_WEB_LISTENERS_H
#define EMP_WEB_LISTENERS_H


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../tools/string_utils.hpp"

#include <map>
#include <string>

namespace emp {
namespace web {

  /// Track a set of JavaScript Listeners with their callback IDs.
  class Listeners {
  private:
    std::map<std::string, size_t> listeners;  ///< Map triggers to callback IDs

  public:
    Listeners() { ; }
    Listeners(const Listeners &) = default;
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

    const std::map<std::string, size_t> & GetMap() const {
      return listeners;
    }

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
      MAIN_THREAD_EM_ASM({
          var id = UTF8ToString($0);
          emp_i.cur_obj = $( '#' + id );
        }, widget_id.c_str());
#endif

      for (auto event_pair : listeners) {
#ifdef __EMSCRIPTEN__
        MAIN_THREAD_EM_ASM({
          var name = UTF8ToString($0);
          emp_i.cur_obj.on( name, function(evt) { emp.Callback($1, evt); } );
        }, event_pair.first.c_str(), event_pair.second);
#else
        std::cout << "Setting '" << widget_id << "' listener '" << event_pair.first
                  << "' to '" << event_pair.second << "'.";
#endif
      }
    }


    /// Apply a SPECIFIC listener.
    static void Apply(const std::string & widget_id,
                      const std::string event_name,
                      size_t fun_id) {
#ifdef __EMSCRIPTEN__
        MAIN_THREAD_EM_ASM({
          var id = UTF8ToString($0);
          var name = UTF8ToString($1);
          $( '#' + id ).on( name, function(evt) { emp.Callback($2, evt); } );
        }, widget_id.c_str(), event_name.c_str(), fun_id);
#else
        std::cout << "Setting '" << widget_id << "' listener '" << event_name
                  << "' to function id '" << fun_id << "'.";
#endif
    }

    /// true/false : do any listeners exist?
    operator bool() const { return (bool) listeners.size(); }
  };


}
}


#endif
