/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  Style.h
 *  @brief A CSS class for tracking font style, etc.
 */


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

  ///  Class to maintain a map of setting names to values that can be easily ported
  ///  over to JavaScript.  A companial class, Attributes, also exists.
  class Style {
  private:
    std::map<std::string, std::string> settings;  ///< CSS Setting values being tracked.

  public:
    Style() { ; }
    Style(const Style &) = default;
    Style(Style &&) = default;
    Style & operator=(const Style &) = default;
    Style & operator=(Style &&) = default;

    /// Return a count of the number of settings that have been set.
    size_t GetSize() const { return settings.size(); }

    Style & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }

    /// Record that setting "s" is set to value "v" (converted to string) and return this object.
    template <typename SET_TYPE>
    Style & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    /// Set all values from in_css here as well.  Return this object.
    Style & Insert(const Style & in_css) {
      settings.insert(in_css.settings.begin(), in_css.settings.end());
      return *this;
    }

    /// Return true/false based on whether "setting" has been given a value in this Style.
    bool Has(const std::string & setting) const {
      return settings.find(setting) != settings.end();
    }

    /// Return the (string) value of "setting" that has been recorded in this Style.
    const std::string & Get(const std::string & setting) const {
      auto it = settings.find(setting);
      return (it == settings.end()) ? emp::empty_string() : it->second;
    }

    const std::map<std::string, std::string> & GetMap() const {
      return settings;
    }

    /// Remove all setting values.
    void Clear() { settings.clear(); }

    /// Remove a specific setting value.
    void Remove(const std::string & setting) {
      settings.erase(setting);
    }

    /// Apply ALL of the style settings to a specified widget.
    void Apply(const std::string & widget_id) {
      // Stop immediately if nothing to set.
      if (settings.size() == 0) return;

      // Find the current object only once.
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          emp_i.cur_obj = $( '#' + id );
        }, widget_id.c_str());
#endif

      for (auto css_pair : settings) {
        if (css_pair.second == "") continue; // Ignore empty entries.
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
            var name = UTF8ToString($0);
            var value = UTF8ToString($1);
            emp_i.cur_obj.css( name, value);
          }, css_pair.first.c_str(), css_pair.second.c_str());
#else
        std::cout << "Setting '" << widget_id << "' attribute '" << css_pair.first
                  << "' to '" << css_pair.second << "'.";
#endif
      }
    }

    /// Apply only a SPECIFIC style setting from the setting library.
    void Apply(const std::string & widget_id, const std::string & setting) {
      emp_assert(Has(setting));

#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value = UTF8ToString($2);
          $( '#' + id ).css( setting, value);
        }, widget_id.c_str(), setting.c_str(), settings[setting].c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << settings[setting] << "'.";
#endif
    }

    /// Apply onlay a SPECIFIC style setting with a specifid value!
    static void Apply(const std::string & widget_id, const std::string & setting,
                      const std::string & value) {
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value = UTF8ToString($2);
          $( '#' + id ).css( setting, value);
        }, widget_id.c_str(), setting.c_str(), value.c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << value << "'.";
#endif
    }

    /// Have any settings be set?
    operator bool() const { return (bool) settings.size(); }
  };


}
}


#endif
