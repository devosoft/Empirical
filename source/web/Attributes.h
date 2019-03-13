/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  Attributes.h
 *  @brief An Attributes class for tracking non-style features about HTML objects
 */

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

  ///  @brief Maintains a map of attribute names to values for use in JavaScript
  ///  Closely related to Style.h, which is for CSS-values.

  class Attributes {
  private:
    std::map<std::string, std::string> settings;

  public:
    Attributes() { ; }
    Attributes(const Attributes &) = default;
    Attributes & operator=(const Attributes &) = default;

    /// Return a count of the number of attributes that have been set.
    int GetSize() const { return (int) settings.size(); }

    Attributes & DoSet(const std::string & in_set, const std::string & in_val) {
      settings[in_set] = in_val;
      return *this;
    }

    /// Record that attribute "a" is set to value "v" (converted to string) and return this object.
    template <typename SET_TYPE>
    Attributes & Set(const std::string & s, SET_TYPE v) {
      return DoSet(s, emp::to_string(v));
    }

    /// Set all values from in_attr here as well.  Return this object.
    Attributes & Insert(const Attributes & in_attr) {
      settings.insert(in_attr.settings.begin(), in_attr.settings.end());
      return *this;
    }

    /// Return true/false based on whether "setting" has been given a value in this Attributes obj.
    bool Has(const std::string & setting) const {
      return settings.find(setting) != settings.end();
    }

    /// Return the (string) value of "setting" that has been recorded in this Attributes obj.
    const std::string & Get(const std::string & setting) const {
      // Note: if setting did not exist, this does return (but not create) an empty entry.
      auto it = settings.find(setting);
      return (it == settings.end()) ? emp::empty_string() : it->second;
    }

    const std::map<std::string, std::string> & GetMap() const {
      return settings;
    }

    void Remove(const std::string & setting) {
      settings.erase(setting);
    }

    /// Remove all setting values.
    void Clear() { settings.clear(); }

    /// Apply ALL of the Attribute's settings to dom element "widget_id".
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

      for (auto attr_pair : settings) {
        if (attr_pair.second == "") continue; // Ignore empty entries.
#ifdef EMSCRIPTEN
        EM_ASM_ARGS({
            var name = UTF8ToString($0);
            var value = UTF8ToString($1);
            emp_i.cur_obj.attr( name, value);
          }, attr_pair.first.c_str(), attr_pair.second.c_str());
#else
        std::cout << "Setting '" << widget_id << "' attribute '" << attr_pair.first
                  << "' to '" << attr_pair.second << "'.";
#endif
      }
    }

    /// Apply onlay a SPECIFIC attributes setting from the setting library to widget_id.
    void Apply(const std::string & widget_id, const std::string & setting) {
      emp_assert(Has(setting));

#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value = UTF8ToString($2);
          $( '#' + id ).attr( setting, value);
        }, widget_id.c_str(), setting.c_str(), settings[setting].c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << settings[setting] << "'.";
#endif
    }

    /// Apply onlay a SPECIFIC attributes setting with a specifid value!
    static void Apply(const std::string & widget_id, const std::string & setting,
                      const std::string & value) {
#ifdef EMSCRIPTEN
      EM_ASM_ARGS({
          var id = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value = UTF8ToString($2);
          $( '#' + id ).attr( setting, value);
        }, widget_id.c_str(), setting.c_str(), value.c_str());
#else
      std::cout << "Setting '" << widget_id << "' attribute '" << setting
                << "' to '" << value << "'.";
#endif
    }

    /// Convert to true if there are any setting, false otherwise.
    operator bool() const { return (bool) settings.size(); }
  };


}
}


#endif
