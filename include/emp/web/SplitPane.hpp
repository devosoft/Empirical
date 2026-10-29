/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/SplitPane.hpp
 * @brief Owned pointer/keyboard resizing for an existing pair of panes.
 *
 * Use example, after the container, first pane, and divider are mounted:
 *
 *   emp::web::SplitPane split;
 *   bool bound = split.Bind("panes", "first-pane", "divider");
 *
 * Keep split alive while resizing is enabled. Caller CSS uses the container's
 * --emp-first-pane-size property to size the first pane; its default value is 50%.
 * SplitPaneOptions configures the property name, orientation, bounds, and step.
 * Drag the divider, use arrow keys (Shift for larger steps), or use Home/End for
 * the bounds. Double-click restores the initial size. Reset() and destruction
 * remove listeners; call Bind() again after replacing the container or divider.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_SPLIT_PANE_HPP_GUARD
#define INCLUDE_EMP_WEB_SPLIT_PANE_HPP_GUARD

#include "init.hpp"

namespace emp::web {
  struct SplitPaneOptions {
    bool vertical = false; ///< false: panes side by side; true: panes stacked.
    double minimum = 10;   ///< Percentage of the container allocated to the first pane.
    double maximum = 90;
    double initial = 50;
    double step = 2;
    std::string css_property = "--emp-first-pane-size";
  };

  /// The caller's CSS uses the custom property to size the first pane.
  /// Bind after mounting; rebind after replacing the container. No global listeners remain on disposal.
  class SplitPane {
    size_t token = 0;
  public:
    SplitPane() = default;
    SplitPane(const SplitPane &) = delete;
    SplitPane & operator=(const SplitPane &) = delete;
    ~SplitPane() { Reset(); }
    void Reset() {
      MAIN_THREAD_EM_ASM({
        const bindings = emp_i.splitPanes;
        bindings?.get($0)?.abort(); bindings?.delete($0);
      }, token);
      token = 0;
    }
    bool Bind(const std::string & container_id, const std::string & first_pane_id,
              const std::string & divider_id, const SplitPaneOptions & options = {}) {
      emp_assert(options.minimum >= 0 && options.maximum <= 100
                 && options.minimum <= options.maximum && options.step > 0);
      Reset();
      token = MAIN_THREAD_EM_ASM_INT({
        const container = document.getElementById(UTF8ToString($0));
        const first = document.getElementById(UTF8ToString($1));
        const divider = document.getElementById(UTF8ToString($2));
        if (!container || !first || !divider) return false;
        const bindings = emp_i.splitPanes || (emp_i.splitPanes = new Map());
        divider.empSplitPane?.abort();
        const controller = new AbortController();
        divider.empSplitPane = controller;
        const token = emp_i.nextSplitPane = (emp_i.nextSplitPane || 0) + 1;
        bindings.set(token, controller);
        const listen = (type, fn) => divider.addEventListener(type, fn, {signal:controller.signal});
        const vertical = !!$3;
        const property = UTF8ToString($8);
        let value;
        const set = next => {
          value = Math.min($5, Math.max($4, next));
          container.style.setProperty(property, value + '%');
          divider.setAttribute('aria-valuenow', String(value));
        };
        divider.tabIndex = 0; divider.setAttribute('role', 'separator');
        divider.setAttribute('aria-orientation', vertical ? 'horizontal' : 'vertical');
        divider.setAttribute('aria-controls', first.id);
        divider.setAttribute('aria-valuemin', String($4));
        divider.setAttribute('aria-valuemax', String($5));
        divider.style.touchAction = 'none';
        set($6);
        listen('pointerdown', event => {
          if (event.button !== 0) return;
          event.preventDefault(); divider.focus(); divider.setPointerCapture(event.pointerId);
        });
        listen('pointermove', event => {
          if (!divider.hasPointerCapture(event.pointerId)) return;
          const bounds = container.getBoundingClientRect();
          const extent = vertical ? bounds.height : bounds.width;
          if (extent > 0) set(100 * (vertical ? event.clientY - bounds.top
                                            : event.clientX - bounds.left) / extent);
        });
        const release = event => {
          if (divider.hasPointerCapture(event.pointerId)) divider.releasePointerCapture(event.pointerId);
        };
        listen('pointerup', release); listen('pointercancel', release);
        listen('keydown', event => {
          const step = $7 * (event.shiftKey ? 5 : 1);
          if (event.key === 'Home') set($4);
          else if (event.key === 'End') set($5);
          else if (event.key === (vertical ? 'ArrowUp' : 'ArrowLeft')) set(value - step);
          else if (event.key === (vertical ? 'ArrowDown' : 'ArrowRight')) set(value + step);
          else return;
          event.preventDefault();
        });
        listen('dblclick', () => set($6));
        return token;
      }, container_id.c_str(), first_pane_id.c_str(), divider_id.c_str(), options.vertical,
         options.minimum, options.maximum, options.initial, options.step, options.css_property.c_str());
      return token != 0;
    }
  };
}

#endif  // INCLUDE_EMP_WEB_SPLIT_PANE_HPP_GUARD
