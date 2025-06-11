/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2017 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Attributes.hpp
 * @brief An Attributes class for tracking non-style features about HTML objects
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_ATTRIBUTES_HPP_GUARD
#define INCLUDE_EMP_WEB_ATTRIBUTES_HPP_GUARD


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif  // #ifdef __EMSCRIPTEN__

#include <map>
#include <string>

#include "../base/errors.hpp"
#include "../tools/string_utils.hpp"

namespace emp { namespace web {

  ///  @brief Maintains a map of attribute names to values for use in JavaScript
  ///  Closely related to Style.h, which is for CSS-values.

  class Attributes {
  private:
    std::map<std::string, std::string> settings;

  public:
    Attributes() { ; }

    Attributes(const Attributes &)             = default;
    Attributes & operator=(const Attributes &) = default;

    /// Return a count of the number of attributes that have been set.
    int GetSize() const { return (int) settings.size(); }

    /// Record that attribute "a" is set to value "v" (converted to string) and return this object.
    template <typename SET_TYPE>
    Attributes & Set(const std::string & s, SET_TYPE v) {
      settings[s] = emp::to_string(v);
      return *this;
    }

    /// Append a new value to an existing attribute
    /// @param in_attr attribute name
    /// @param in_val attribute value to be added
    Attributes & DoAddAttr(const std::string in_attr, const std::string & in_val) {
      if (!Has(in_attr)) {
        // Attribute has not been assigned to this Widget. Add it with Set instead
        Set(in_attr, in_val);
      } else if (settings[in_attr].find(in_val) == std::string::npos) {
        // New value is not a duplicate of any values assigned to this attribute. Append it.
        settings[in_attr] += " " + in_val;
      }
      return *this;
    }

    std::string GetAttrValue(const std::string & in_set) { return settings[in_set]; }

    /// Set all values from in_attr here as well.  Return this object.
    Attributes & Insert(const Attributes & in_attr) {
      settings.insert(in_attr.settings.begin(), in_attr.settings.end());
      return *this;
    }

    /// Return true/false based on whether "setting" has been given a value in this Attributes obj.
    bool Has(const std::string & setting) const { return settings.find(setting) != settings.end(); }

    /// Return the (string) value of "setting" that has been recorded in this Attributes obj.
    const std::string & Get(const std::string & setting) const {
      // Note: if setting did not exist, this does return (but not create) an empty entry.
      auto it = settings.find(setting);
      return (it == settings.end()) ? emp::empty_string() : it->second;
    }

    const std::map<std::string, std::string> & GetMap() const { return settings; }

    void Remove(const std::string & setting) { settings.erase(setting); }

    /// Remove all setting values.
    void Clear() { settings.clear(); }

    /// Apply ALL of the Attribute's settings to dom element "widget_id".
    void Apply(const std::string & widget_id) {
      // Stop immediately if nothing to set.
      if (settings.size() == 0) { return; }

      // Find the current object only once.
#ifdef __EMSCRIPTEN__
      MAIN_THREAD_EM_ASM(
        {
          var id        = UTF8ToString($0);
          emp_i.cur_obj = document.getElementById(id);
        },
        widget_id.c_str());
#endif  // #ifdef __EMSCRIPTEN__

      for (auto attr_pair : settings) {
        if (attr_pair.second == "") {
          continue;  // Ignore empty entries.
        }
#ifdef __EMSCRIPTEN__
        MAIN_THREAD_EM_ASM(
          {
            var name  = UTF8ToString($0);
            var value = UTF8ToString($1);
            if (emp_i.cur_obj) { emp_i.cur_obj.setAttribute(name, value); }
          },
          attr_pair.first.c_str(),
          attr_pair.second.c_str());
#else   // #ifdef __EMSCRIPTEN__
        std::cout << "Setting '" << widget_id << "' attribute '" << attr_pair.first << "' to '"
                  << attr_pair.second << "'.";
#endif  // #ifdef __EMSCRIPTEN__ : #else
      }
    }

    /// Apply only a SPECIFIC attributes setting from the setting library to widget_id.
    void Apply(const std::string & widget_id, const std::string & setting) {
      emp_assert(Has(setting));

#ifdef __EMSCRIPTEN__
      MAIN_THREAD_EM_ASM(
        {
          var id      = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value   = UTF8ToString($2);
          var element = document.getElementById(id);
          if (element) { element.setAttribute(setting, value); }
        },
        widget_id.c_str(),
        setting.c_str(),
        settings[setting].c_str());
#else   // #ifdef __EMSCRIPTEN__
      std::cout << "Setting '" << widget_id << "' attribute '" << setting << "' to '"
                << settings[setting] << "'.";
#endif  // #ifdef __EMSCRIPTEN__ : #else
    }

    /// Apply only a SPECIFIC attributes setting with a specified value!
    static void Apply(const std::string & widget_id,
                      const std::string & setting,
                      const std::string & value) {
#ifdef __EMSCRIPTEN__
      MAIN_THREAD_EM_ASM(
        {
          var id      = UTF8ToString($0);
          var setting = UTF8ToString($1);
          var value   = UTF8ToString($2);
          var element = document.getElementById(id);
          if (element) { element.setAttribute(setting, value); }
        },
        widget_id.c_str(),
        setting.c_str(),
        value.c_str());
#else   // #ifdef __EMSCRIPTEN__
      std::cout << "Setting '" << widget_id << "' attribute '" << setting << "' to '" << value
                << "'.";
#endif  // #ifdef __EMSCRIPTEN__ : #else
    }

    /// Convert to true if there are any setting, false otherwise.
    operator bool() const { return (bool) settings.size(); }
  };


}}  // namespace emp::web


#endif  // #ifndef INCLUDE_EMP_WEB_ATTRIBUTES_HPP_GUARD
