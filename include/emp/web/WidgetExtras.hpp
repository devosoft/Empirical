/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
 *
 *  @file  WidgetExtras.hpp
 *  @brief A collection of extra details about HTML Widgets (attributes, style, listerns)
 */

#ifndef EMP_WEB_WIDGET_EXTRA_H
#define EMP_WEB_WIDGET_EXTRA_H

#include "Attributes.hpp"
#include "init.hpp"
#include "Listeners.hpp"
#include "Style.hpp"

namespace emp {
namespace web {

  struct WidgetExtras {
    Style style;       ///< CSS Style
    Attributes attr;   ///< HTML Attributes about a cell.
    Listeners listen;  ///< Listen for web events

    template <typename SET_TYPE>
    void SetStyle(const std::string & s, SET_TYPE v) { style.Set(s, emp::to_string(v)); }
    bool HasStyle(const std::string & setting) const { return style.Has(setting); }
    const std::string & GetStyle(const std::string & setting) const { return style.Get(setting); }
    void RemoveStyle(const std::string & setting) { style.Remove(setting); }

    template <typename SET_TYPE>
    void SetAttr(const std::string & s, SET_TYPE v) { attr.Set(s, emp::to_string(v)); }
    bool HasAttr(const std::string & setting) const { return attr.Has(setting); }
    const std::string & GetAttr(const std::string & setting) const { return attr.Get(setting); }
    void RemoveAttr(const std::string & setting) { attr.Remove(setting); }

    /// Apply all HTML details associated with this widget.
    void Apply(const std::string & name) {
      style.Apply(name);
      attr.Apply(name);
      listen.Apply(name);
    }

    /// Clear all of style, attributes, and listeners.
    void Clear() {
      style.Clear();
      attr.Clear();
      listen.Clear();
    }

    /// Have any details been set?
    operator bool() const { return style || attr || listen; } // Return true if any extras are set.
  };

}
}

#endif
