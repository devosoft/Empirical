//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  An Attributes class for tracking non-style features about HTML objects
//
//  This class maintains a map of attribute names to values that can be easily ported
//  over to JavaScript.  It is closely related to Style.h, which is for CSS-values.
//
//  int GetSize() const
//    Return a count of the number of attributes that have been set.
//
//  Attributes & Set(const std::string & a, SET_TYPE v)
//    Record that attribute "a" is set to value "v" (converted to string) and return this object.
//
//  Attributes & Insert(const Attributes & in_attr)
//    Set all values from in_attr here as well.  Return this object.
//
//  bool Has(const std::string setting) const
//    Return true/false based on whether "setting" has been given a value in this Attributes obj.
//
//  const std::string & Get(const std::string setting)
//    Return the (string) value of "setting" that has been recorded in this Attributes obj.
//    If setting did not exist, this does create an empty entry and return it.
//
//  void Clear()
//    Remove all setting values.
//
//  void Apply(const std::string & widget_id) -
//    Apply all settings to dom element "widget_id".
//
//  void Apply(const std::string & widget_id, const std::string & setting)
//    Apply current value of "setting" to dom element "widget_id".
//
//  static void Apply(const std::string & widget_id, const std::string & setting,
//                    const std::string & value)
//    Apply "setting: value" to widget_id.


#ifndef EMP_WEB_ATTRIBUTES_H
#define EMP_WEB_ATTRIBUTES_H


#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "../tools/string_utils.h"

#include <map>
#include <string>

namespace emp {
namespace web {

  class Attributes {
  private:
    std::map<std::string, std::string> settings;

  public:
    Attributes() { ; }
    Attributes(const Attributes &) = default;
    Attributes & operator=(const Attributes &) = default;

    int GetSize() const { return (int) settings.size(); }

    Attributes & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }

    template <typename SET_TYPE>
    Attributes & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    Attributes & Insert(const Attributes & in_attr) {
      settings.insert(in_attr.settings.begin(), in_attr.settings.end());
      return *this;
    }

    bool Has(const std::string & setting) const {
      return settings.find(setting) != settings.end();
    }

    const std::string & Get(const std::string & setting) {
      // Note: if setting did not exist, this does create an empty entry.
      return settings[setting];
    }

    const std::map<std::string, std::string> & GetMap() const {
      return settings;
    }

    void Remove(const std::string & setting) {
      settings.erase(setting);
    }

    void Clear() { settings.clear(); }

    // Apply ALL of the Attributes settings.
    void Apply(const std::string & widget_id) {
      // Stop immediately if nothing to set.
      if (settings.size() == 0) return;

      // Find the current object only once.
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          emp_i.cur_obj = $( '#' + id );
        }, widget_id.c_str());
#endif

      for (auto attr_pair : settings) {
        if (attr_pair.second == "") continue; // Ignore empty entries.
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
            var name = Pointer_stringify($0);
            var value = Pointer_stringify($1);
            emp_i.cur_obj.attr( name, value);
          }, attr_pair.first.c_str(), attr_pair.second.c_str());
#else
        std::cout << "Setting '" << widget_id << "' attribute '" << attr_pair.first
                  << "' to '" << attr_pair.second << "'.";
#endif
      }
    }

    // Apply onlay a SPECIFIC attributes setting from the setting library.
    void Apply(const std::string & widget_id, const std::string & setting) {
      emp_assert(Has(setting));

#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          var setting = Pointer_stringify($1);
          var value = Pointer_stringify($2);
          $( '#' + id ).attr( setting, value);
        }, widget_id.c_str(), setting.c_str(), settings[setting].c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << settings[setting] << "'.";
#endif
    }

    // Apply onlay a SPECIFIC attributes setting with a specifid value!
    static void Apply(const std::string & widget_id, const std::string & setting,
                      const std::string & value) {
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          var setting = Pointer_stringify($1);
          var value = Pointer_stringify($2);
          $( '#' + id ).attr( setting, value);
        }, widget_id.c_str(), setting.c_str(), value.c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << value << "'.";
#endif
    }

  };


}
}


#endif
