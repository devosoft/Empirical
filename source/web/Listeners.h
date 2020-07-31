/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  Listeners.h
 *  @brief A class for tracking font event listeners for Widgets
 */


// NEW CODE

#ifndef EMP_WEB_LISTENERS_H
#define EMP_WEB_LISTENERS_H


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../tools/string_utils.h"

#include <map>
#include <string>

namespace emp {
namespace web {

  /// Track a set of JavaScript Listeners with their callback IDs.
  class Listeners {
  private:
    std::map<std::string, std::map<std::string, size_t>> listeners;  ///< Map triggers to list of callback IDs. The outer map associates event names with an inner map, which maps handler IDs to their callbacks.

  public:
    Listeners() { ; }
    Listeners(const Listeners &) = default;
    Listeners & operator=(const Listeners &) = default;

    /// How many listeners are we tracking? 
    // Should we just store this?
    size_t GetSize() const {
      size_t count = 0;
      for (auto event_pair : listeners)
        for (auto handler_pair : event_pair.second) {
          count++;
        }
      return count;
     }

    /// Use a pre-calculated function ID with a new listener.
    Listeners & Set(const std::string & event_name, size_t fun_id, std::string handler_id="default") {
      emp_assert(!HasHandler(event_name, handler_id));
      listeners[event_name][handler_id] = fun_id;
      return *this;
    }

    /// Calculate its own function ID with JSWrap.
    template <typename... Ts>
    Listeners & Set(const std::string & event_name, const std::function<void(Ts... args)> & in_fun, std::string handler_id="default") {
      emp_assert(!HasHandler(event_name, handler_id));
      listeners[event_name][handler_id] = JSWrap(in_fun);
      return *this;
    }

    /// Determine if any listener exists with the specified event name.
    bool Has(const std::string & event_name) const {
      return listeners.find(event_name) != listeners.end();
    }

       /// Determine if any listener exists with the specified event name and handler ID.
    bool HasHandler(const std::string & event_name, std::string handler_id = "default") const {
      if (listeners.find(event_name) != listeners.end()) {
        const std::map<std::string, size_t>& m = listeners.at(event_name);
        return m.find(handler_id) != m.end();
      }
      return false;
    }

    /// Get the ID associated with a specific listener. 
    size_t GetID(const std::string & event_name, std::string handler_id = "default") {
      emp_assert(HasHandler(event_name, handler_id));
      return listeners[event_name][handler_id];
    }

    const std::map<std::string, std::map<std::string, size_t>> & GetMap() const {
      return listeners;
    }

    /// Remove all listeners
    void Clear() {
      // @CAO: Delete functions to be called.
      listeners.clear();
    }

    /// Remove all listeners with the given event name.
    void Remove(const std::string & event_name) {
      // @CAO: Delete function to be called.
      listeners.erase(event_name);
    }

    /// Remove the listener with the given event name and handler ID.
    void Remove(const std::string & event_name, std::string handler_id = "default") {
      // @CAO: Delete function to be called.
      if (Has(event_name))
      {
        listeners[event_name].erase(handler_id);
        std::cout << "removed listener " << handler_id << std::endl;
      }
    }

    /// Apply all of the listeners being tracked.
    void Apply(const std::string & widget_id, bool add_before_onclick = false) {
      // Find the current object only once.
#ifdef __EMSCRIPTEN__
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          emp_i.cur_obj = $( '#' + id );
        }, widget_id.c_str());
      if(add_before_onclick){
        EM_ASM({
          var onclick_handler = emp_i.onclick;
          emp_i.removeProp('onclick');
        });
      }
#endif

      for (auto event_pair : listeners) {
        for (auto handler_pair : event_pair.second) {
          size_t & fun_id = handler_pair.second;
          #ifdef __EMSCRIPTEN__
            EM_ASM_ARGS({
              var name = UTF8ToString($0);
              emp_i.cur_obj.on( name, function(evt) { emp.Callback($1, evt); } );
            }, event_pair.first.c_str(), fun_id);
          if(add_before_onclick){
            EM_ASM({
              emp_i.click(onclick_handler);
            });
          }
          #else
              std::cout << "Setting '" << widget_id << "' listener '" << event_pair.first
                        << "' to '" << fun_id << "'.";
#endif
        }
      }
    }


    /// Apply a SPECIFIC listener.
    static void Apply(const std::string & widget_id,
                      const std::string event_name,
                      size_t fun_id, bool add_before_onclick = false) {
#ifdef __EMSCRIPTEN__
        EM_ASM_ARGS({
          var id = UTF8ToString($0);
          var name = UTF8ToString($1);
          if($3){
            var onclick_handler = $('#' + id).prop('onclick');
            if(onclick_handler){
              $('#' + id).prop('onclick', null);
              alert('bumping onclick back ' + '$(#' + id + ').prop(onclick, null)');
            }
          }
          $( '#' + id ).on( name, function(evt) { emp.Callback($2, evt); } );
          if($3){
            if(onclick_handler){
              //$('#' + id).click(onclick_handler);
              //$('#' + id).on('click', onclick_handler);
              //$('#' + id).attr('onclick', 'onclick_handler();');
            }
          }
        }, widget_id.c_str(), event_name.c_str(), fun_id, add_before_onclick);
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
