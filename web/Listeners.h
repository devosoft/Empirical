//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A class for tracking font event listeners for Widgets


#ifndef EMP_WEB_LISTENERS_H
#define EMP_WEB_LISTENERS_H


#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "../tools/string_utils.h"

#include <map>
#include <string>

namespace emp {
namespace web {

  class Listeners {
  private:
    std::map<std::string, size_t> listeners;  // Map triggers to callback IDs

  public:
    Listeners() { ; }
    Listeners(const Listeners &) = default;
    Listeners & operator=(const Listeners &) = default;

    size_t GetSize() const { return listeners.size(); }

    template <typename... Ts>
    Listeners & Set(const std::string & name, const std::function<void(Ts... args)> & in_fun) {
      emp_assert(!Has(name));
      listeners[name] = JSWrap(in_fun);
      return *this;
    }


    bool Has(const std::string & event_name) const {
      return listeners.find(event_name) != listeners.end();
    }

    size_t GetID(const std::string & event_name) {
      emp_assert(Has(event_name));
      return listeners[event_name];
    }

    const std::map<std::string, size_t> & GetMap() const {
      return listeners;
    }

    void Clear() {
      // @CAO: Delete functions to be called.
      listeners.clear();
    }

    void Remove(const std::string & event_name) {
      // @CAO: Delete function to be called.
      listeners.erase(event_name);
    }

    // Apply all of the listeners.
    void Apply(const std::string & widget_id) {
      // Find the current object only once.
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          emp_i.cur_obj = $( '#' + id );
        }, widget_id.c_str());
#endif

      for (auto event_pair : listeners) {
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
          var name = Pointer_stringify($0);
          emp_i.cur_obj.on( name, function(evt) { emp.Callback($1, evt); } );
        }, event_pair.first.c_str(), event_pair.second);
#else
        std::cout << "Setting '" << widget_id << "' listener '" << event_pair.first
                  << "' to '" << event_pair.second << "'.";
#endif
      }
    }

  };


}
}


#endif
