//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Extra details about HTML Widgets.
//
//  Includes:
//  * Widget Attributes
//  * CSS Style
//  * Listeners for user interaction

#ifndef EMP_WEB_WIDGET_EXTRA_H
#define EMP_WEB_WIDGET_EXTRA_H

#include "Attributes.h"
#include "init.h"
#include "Listeners.h"
#include "Style.h"

namespace emp {
namespace web {

  struct WidgetExtras {
    Style style;       // CSS Style
    Attributes attr;   // HTML Attributes about a cell.
    Listeners listen;  // Listen for web events

    bool IsAnnotated() { return style || attr || listen; }
    void Apply(const std::string & name) {
      style.Apply(name);
      attr.Apply(name);
      listen.Apply(name);
    }
  };

}
}

#endif


