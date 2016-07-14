//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A CSS class for tracking font style, etc.
//
//  This class maintains a map of setting names to values that can be easily ported
//  over to JavaScript.  A companial class, Attributes, also exists.
//
//  int GetSize() const
//    Return a count of the number of settings that have been set.
//
//  Style & Set(const std::string & s, SET_TYPE v)
//    Record that setting "s" is set to value "v" (converted to string) and return this object.
//
//  Style & Insert(const Style & in_css)
//    Set all values from in_css here as well.  Return this object.
//
//  bool Has(const std::string setting) const
//    Return true/false based on whether "setting" has been given a value in this Style.
//
//  const std::string & Get(const std::string setting)
//    Return the (string) value of "setting" that has been recorded in this Style.
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


#ifndef EMP_WEB_STYLE_H
#define EMP_WEB_STYLE_H


#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "../tools/string_utils.h"

#include <map>
#include <string>

namespace emp {
namespace web {

  class Style {
  private:
    std::map<std::string, std::string> settings;

  public:
    Style() { ; }
    Style(const Style &) = default;
    Style & operator=(const Style &) = default;

    int GetSize() const { return (int) settings.size(); }

    Style & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }

    template <typename SET_TYPE>
    Style & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    Style & Insert(const Style & in_css) {
      settings.insert(in_css.settings.begin(), in_css.settings.end());
      return *this;
    }

    bool Has(const std::string setting) const {
      return settings.find(setting) != settings.end();
    }

    const std::string & Get(const std::string setting) {
      // Note: if setting did not exist, this does create an empty entry.
      return settings[setting];
    }

    const std::map<std::string, std::string> & GetMap() const {
      return settings;
    }

    void Clear() { settings.clear(); }

    // Apply ALL of the style settings.
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

      for (auto css_pair : settings) {
        if (css_pair.second == "") continue; // Ignore empty entries.
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
            var name = Pointer_stringify($0);
            var value = Pointer_stringify($1);
            emp_i.cur_obj.css( name, value);
          }, css_pair.first.c_str(), css_pair.second.c_str());
#else
        std::cout << "Setting '" << widget_id << "' attribute '" << css_pair.first
                  << "' to '" << css_pair.second << "'.";
#endif
      }
    }

    // Apply onlay a SPECIFIC style setting from the setting library.
    void Apply(const std::string & widget_id, const std::string & setting) {
      emp_assert(Has(setting));

#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          var setting = Pointer_stringify($1);
          var value = Pointer_stringify($2);
          $( '#' + id ).css( setting, value);
        }, widget_id.c_str(), setting.c_str(), settings[setting].c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << settings[setting] << "'.";
#endif
    }

    // Apply onlay a SPECIFIC style setting with a specifid value!
    static void Apply(const std::string & widget_id, const std::string & setting,
                      const std::string & value) {
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = Pointer_stringify($0);
          var setting = Pointer_stringify($1);
          var value = Pointer_stringify($2);
          $( '#' + id ).css( setting, value);
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
