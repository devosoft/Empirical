/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Selector.hpp
 * @brief Create a pull-down menu with option values, labels, and change callbacks.
 *
 * Use example with a separate function for each option:
 *
 *   emp::web::Selector actions("actions");
 *   actions.SetOption("Show summary", ShowSummary);
 *   actions.SetOption("Show details", []() { ShowDetails(); });
 *   actions.SetOption("No action");
 *   doc << actions;
 *
 * SetOption(label, callback) appends an option and stores a void() callback to
 * invoke when it is chosen. The callback is optional. To set or replace an option
 * at a specific zero-based index, use SetOption(label, callback, index) or
 * SetOption(label, index). Replacing an option also replaces its callback;
 * overloads without a callback clear the previous per-option action.
 *
 * Use example with explicit values and one callback for the whole selector:
 *
 *   emp::web::Selector mode("mode", "select-control");
 *   mode.AddOption("fast", "Fast mode")
 *       .AddOption("precise", "Precise mode");
 *   mode.Callback(OnModeChange);
 *   mode.SelectValue("fast");
 *   doc << mode;
 *
 * Here doc is a web::Document or another container. The constructor accepts an
 * optional HTML ID and whitespace-separated CSS classes. AddOption(value, label)
 * always appends, separating a stored string value from its display label.
 * OnModeChange receives the selected value as const std::string &. Explicit
 * values are useful for settings whose labels may change or be translated.
 * Newly created SetOption() entries use their index as a string value ("0", "1",
 * etc.); replacing an existing entry preserves its value.
 *
 * Both callback styles are supported. Callback(fn) also applies to entries made
 * with SetOption(). On a user change or TriggerID(index), the per-option callback
 * runs first, followed by the selector-wide callback if one is installed. Adding
 * options does not invoke callbacks. Objects captured by reference must remain
 * alive while their callbacks can run.
 *
 * Selection and inspection:
 *   SelectID(index) selects silently; supply an index below GetNumOptions().
 *   SelectValue(value) selects the first matching value silently, returning false
 *     and leaving the selection unchanged when no match exists.
 *   TriggerID(index) selects and invokes callbacks; invalid indices are ignored.
 *   GetSelectID() returns the selected index; GetValue() returns its stored value.
 *   GetNumOptions() returns the number of entries; GetOption(index) returns the
 *     stored label, including any HTML escaping applied by AddOption().
 *
 * AddOption() treats labels as plain text and escapes them for HTML. SetOption()
 * accepts HTML labels; escape untrusted text before passing it to SetOption().
 * When changing a displayed selector, AddOption() redraws automatically, while
 * SetOption() requires an explicit Redraw() to display the updated options.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_SELECTOR_HPP_GUARD
#define INCLUDE_EMP_WEB_SELECTOR_HPP_GUARD

#include <stddef.h>

#include "../base/vector.hpp"
#include "../tools/String.hpp"

#include "JSWrap.hpp"
#include "Widget.hpp"

namespace emp::web {

  class Selector : public internal::WidgetFacet<Selector> {
    friend class SelectorInfo;
  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    class SelectorInfo : public internal::WidgetInfo {
      friend Selector;
    protected:
      emp::vector<std::string> options;                 ///< What are the options to choose from?
      emp::vector<std::string> values;
      std::function<void(const std::string &)> on_change;
      emp::vector<std::function < void()> > callbacks;  ///< Which function to run for each option?
      size_t select_id;                                 ///< Which index is currently selected?

      bool autofocus;
      bool disabled;

      size_t callback_id;

      SelectorInfo(const std::string & in_id = "")
        : internal::WidgetInfo(in_id), select_id(0), autofocus(false), disabled(false) {
        ;
      }

      SelectorInfo(const SelectorInfo &)             = delete;  // No copies of INFO allowed
      SelectorInfo & operator=(const SelectorInfo &) = delete;  // No copies of INFO allowed

      virtual ~SelectorInfo() {
        if (callback_id) {
          emp::JSDelete(callback_id);  // Delete callback wrapper.
        }
      }

      std::string GetTypeName() const override { return "SelectorInfo"; }

      void SetOption(const std::string & name, const std::function<void()> & cb, size_t id) {
        // If we need more room for options, increase the array size.
        if (id >= options.size()) {
          options.resize(id + 1);
          callbacks.resize(id + 1);
          const size_t previous_size = values.size();
          values.resize(id + 1);
          for (size_t i = previous_size; i < values.size(); ++i) { values[i] = emp::to_string(i); }
        }
        options[id]   = name;
        callbacks[id] = cb;
      }

      void SetOption(const std::string & name, const std::function<void()> & cb) {
        SetOption(name, cb, options.size());  // No option id specified, so choose the next one.
      }

      void DoChange(size_t new_id) {
        Widget keep_alive(this); // User callbacks may remove the final external widget handle.
        if (new_id >= options.size()) { return; }
        select_id = new_id;
        if (callbacks[new_id]) { callbacks[new_id](); }
        if (on_change) { on_change(values[new_id]); }
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML << "<select";                           // Start the select tag.
        if (disabled) { HTML << " disabled=true"; }  // Check if should be disabled
        HTML << " id=\"" << id << "\"";              // Indicate ID.

        // Indicate action on change.
        HTML << " onchange=\"emp.Callback(" << callback_id << ", this.selectedIndex)\">";

        // List out options
        for (size_t i = 0; i < options.size(); i++) {
          HTML << "<option value=\"" << emp::MakeWebSafe(values[i]) << "\"";
          if (i == select_id) { HTML << " selected"; }
          HTML << ">" << options[i] << "</option>";
        }
        HTML << "</select>";
      }

      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) { ReplaceHTML(); }
      }

      void UpdateDisabled(bool in_dis) {
        disabled = in_dis;
        if (state == Widget::ACTIVE) { ReplaceHTML(); }
      }

    public:
      virtual std::string GetType() override { return "web::SelectorInfo"; }
    };  // End of SelectorInfo class.
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Get a properly cast version of info.
    SelectorInfo * Info() { return (SelectorInfo *) info; }

    const SelectorInfo * Info() const { return (SelectorInfo *) info; }

    Selector(SelectorInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    Selector(const std::string & in_id = "", const std::string & css_class = "") : WidgetFacet(in_id) {
      info = new SelectorInfo(in_id);
      if (!css_class.empty()) { AddClass(css_class); }

      Info()->select_id = 0;
      Info()->autofocus = false;
      Info()->disabled  = false;

      SelectorInfo * s_info = Info();
      Info()->callback_id =
        JSWrap(std::function<void(size_t)>([s_info](size_t new_id) { s_info->DoChange(new_id); }));
    }

    Selector(const Selector & in) : WidgetFacet(in) { ; }

    Selector(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsSelector()); }

    virtual ~Selector() { ; }

    using INFO_TYPE = SelectorInfo;

    /// Get the ID of the currently active selection.
    size_t GetSelectID() const { return Info()->select_id; }

    /// Get the total number of options setup in the selector.
    size_t GetNumOptions() const { return Info()->options.size(); }

    /// Get the label associated with a specific option ID.
    const std::string & GetOption(size_t id) const { return Info()->options[id]; }

    /// Determine if the selector has autofocus.
    bool HasAutofocus() const { return Info()->autofocus; }

    /// Is the selector currently disabled?
    bool IsDisabled() const { return Info()->disabled; }

    /// Select a valid option index without invoking callbacks; update the live control if active.
    Selector & SelectID(size_t id) {
      Info()->select_id = id;
      if (IsActive()) {
        MAIN_THREAD_EM_ASM({
          const control = document.getElementById(UTF8ToString($0));
          if (control) control.selectedIndex = $1;
        }, GetID().c_str(), id);
      }
      return *this;
    }

    /// Append an option with an explicit string value and escaped plain-text label; redraw if active.
    Selector & AddOption(const std::string & value, const std::string & label) {
      Info()->SetOption(emp::MakeWebSafe(label), {});
      Info()->values.back() = value;
      if (IsActive()) { Info()->ReplaceHTML(); }
      return *this;
    }

    /// Set the selector-wide callback, run after any per-option callback with the selected value.
    Selector & Callback(const std::function<void(const std::string &)> & fn) {
      Info()->on_change = fn;
      return *this;
    }

    /// Get the selected value, or an empty string if no valid option is selected.
    const std::string & GetValue() const {
      return GetSelectID() >= Info()->values.size() ? emp::empty_string() : Info()->values[GetSelectID()];
    }

    /// Select silently by value; leave the selection unchanged if the value is absent.
    bool SelectValue(const std::string & value) {
      for (size_t i = 0; i < Info()->values.size(); ++i) {
        if (Info()->values[i] == value) { SelectID(i); return true; }
      }
      return false;
    }

    /// Select an index and invoke both callback styles; ignore indices outside the option range.
    Selector & TriggerID(size_t id) {
      Info()->DoChange(id);
      return *this;
    }

    /// Append an HTML label and its per-option callback; call Redraw() if already displayed.
    Selector & SetOption(const std::string & in_option, const std::function<void()> & in_cb) {
      Info()->SetOption(in_option, in_cb);
      return *this;
    }

    /// Set or replace an HTML label and callback at an index, preserving any existing value.
    /// Extend the option list if needed; call Redraw() if already displayed.
    Selector & SetOption(const std::string & in_option,
                         const std::function<void()> & in_cb,
                         size_t opt_id) {
      Info()->SetOption(in_option, in_cb, opt_id);
      return *this;
    }

    /// Set a selector option name, but no function to be called.
    Selector & SetOption(const std::string & in_option) {
      return SetOption(in_option, std::function<void()>([]() {}));
    }

    /// Set a specific selection option name, determined by the ID, but no function to call.
    Selector & SetOption(const std::string & in_option, size_t opt_id) {
      return SetOption(in_option, std::function<void()>([]() {}), opt_id);
    }

    /// Update autofocus setting.
    Selector & Autofocus(bool in_af) {
      Info()->UpdateAutofocus(in_af);
      return *this;
    }

    /// Update disabled status.
    Selector & Disabled(bool in_dis) {
      Info()->UpdateDisabled(in_dis);
      return *this;
    }
  };


}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_SELECTOR_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: sel
