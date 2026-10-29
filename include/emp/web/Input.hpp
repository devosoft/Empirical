/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2018 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/Input.hpp
 * @brief Create an HTML input with change callbacks, silent updates, and validation.
 *
 * Use example:
 *
 *   emp::web::Input my_input(MyFun, "number", "Amount", "amount");
 *   my_input.SetValueSilently("0.25");
 *   doc << my_input;
 *
 * MyFun receives the new value as a std::string when an accepted change occurs.
 * The HTML input type and label follow the callback; the ID is optional. Here doc
 * is a web::Document or another container. SetValueSilently() synchronizes the
 * stored and displayed value without callbacks or replacing the live element.
 * Validator() returns an empty string for success or an error message to reject
 * a change. RejectValue() restores a value and displays validation feedback.
 * Optional CSS classes follow the existing show_value and is_checked arguments.
 *
 * Member functions to set state:
 *   Input & Callback(const std::function<void(std::string)> & in_callback)
 *   Input & Label(const std::string & in_label)
 *   Input & Autofocus(bool in_af)
 *   Input & Disabled(bool in_dis)
 *
 * Retrieving current state:
 *   const std::string & GetCurrValue() const
 *   const std::string & GetValidationError() const
 *   const std::string & GetLabel() const
 *   bool HasAutofocus() const
 *   bool IsDisabled() const
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_INPUT_HPP_GUARD
#define INCLUDE_EMP_WEB_INPUT_HPP_GUARD

#include <cstdint>

#include "../tools/String.hpp"

#include "Widget.hpp"

namespace emp::web {

  /// Create or control an HTML Input object that you can manipulate and update as needed.
  class Input : public internal::WidgetFacet<Input> {
    friend class InputInfo;
  protected:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Inputs associated with the same DOM element share a single InputInfo object.
    class InputInfo : public internal::WidgetInfo {
      friend Input;
    protected:
      std::string label;
      std::string type;
      std::string min   = "";
      std::string max   = "";
      std::string value = "";
      std::string step  = "";

      std::string curr_val = "";
      bool show_value      = false;
      bool autofocus;

      std::function<void(std::string)> callback;
      std::function<bool(std::string)> checker;
      std::function<std::string(const std::string &)> validator;
      std::string validation_error;
      size_t validation_callback_id = 0;
      uint32_t callback_id;
      std::string onchange_info;

      InputInfo(const std::string & in_id = "")
        : internal::WidgetInfo(in_id), checker([](std::string /*in*/) { return true; }) {
        ;
      }

      InputInfo(const InputInfo &)             = delete;  // No copies of INFO allowed
      InputInfo & operator=(const InputInfo &) = delete;  // No copies of INFO allowed

      virtual ~InputInfo() {
        if (validation_callback_id) { emp::JSDelete(validation_callback_id); }
        if (callback_id) {
          emp::JSDelete(callback_id);  // Delete callback wrapper.
        }
      }

      std::string GetTypeName() const override { return "InputInfo"; }

      virtual void GetHTML(std::stringstream & HTML) override {
        // CSS from https://stackoverflow.com/questions/46695616/align-range-slider-and-label

        HTML.str("");  // Clear the current text.
        // HTML << "<form style=\"display:flex; flex-flow:row; align-items:center;\">";                                       // Needs to be part of form for label + output to work
        if (label != "") {  // Add label, if one exists
          HTML << "<label for=\"" << id << "\"> ";
          HTML << label << "</label>";
        }
        HTML << "<input type=\"" << type << "\"";  // Indicate input type.
        if (min != "") {
          HTML << " min=\"" << min << "\"";  // Add a min allowed value if there is one.
        }
        if (max != "") {
          HTML << " max=\"" << max << "\"";  // Add a max allowed value if there is one.
        }
        if (value != "") {
          HTML << " value=\"" << emp::MakeWebSafe(value) << "\"";  // Add a current value if there is one.
        }
        if (step != "") {
          HTML << " step=\"" << step << "\"";  // Add a step if there is one.
        }
        HTML << " id=\"" << id << "\"";                   // Indicate ID.
        HTML << " onchange=\"" << onchange_info << "\"";  // Indicate action on change.
        HTML << ">" << "</input>";                        // Close the Input.
        if (show_value) {
          HTML << "<output for=" << id << "onforminput=\"value = " << id
               << ".valueAsNumber;\"></output>";  // Add output to show value of slider
        }
        // HTML << "</form>";
      }

      virtual void TriggerJS() override {
        ApplyValidation(false);
        if (show_value) {
          // Inspired by https://codepen.io/chriscoyier/pen/imdrE
          EM_ASM_ARGS(
            {
              function modifyOffset() {
                var el;
                var newPlace;
                var offset;
                var siblings;
                var k;
                var width    = this.offsetWidth;
                var newPoint = (this.value - this.getAttribute("min")) /
                               (this.getAttribute("max") - this.getAttribute("min"));
                offset = -1;
                if (newPoint < 0) {
                  newPlace = 0;
                } else if (newPoint > 1) {
                  newPlace = width;
                } else {
                  newPlace = width * newPoint + offset;
                  offset -= newPoint;
                }
                siblings = this.parentNode.childNodes;
                for (var i = 0; i < siblings.length; i++) {
                  sibling = siblings[i];
                  if (sibling.id == this.id) { k = true; }
                  if ((k == true) && (sibling.nodeName == "OUTPUT")) { outputTag = sibling; }
                }
                outputTag.innerHTML = this.value;
              }

              function modifyInputs() {
                var input_el = document.getElementById(UTF8ToString($0));
                if (!input_el) { return; }
                input_el.addEventListener("input", modifyOffset);
                // the following taken from http://stackoverflow.com/questions/2856513/trigger-onchange-event-manually
                if ("fireEvent" in input_el) {
                  input_el.fireEvent("oninput");
                } else {
                  var evt = document.createEvent("HTMLEvents");
                  evt.initEvent("input", false, true);
                  input_el.dispatchEvent(evt);
                }
              }

              modifyInputs();
            },
            id.c_str());
        }
      }

      void DoChange(std::string new_val) {
        Widget keep_alive(this); // User callbacks may remove the final external widget handle.
        if (curr_val == new_val) { return; }
        const std::string error = validator ? validator(new_val) : "";
        if (!checker(new_val) || !error.empty()) {
          UpdateValueSilently(curr_val);
          if (!error.empty()) { SetValidationError(error, true); }
          return;
        }
        SetValidationError("", false);
        value = curr_val = new_val;
        if (callback) { callback(curr_val); }
        UpdateDependants();
      }

      void UpdateValueSilently(const std::string & new_value) {
        value = curr_val = new_value;
        if (type == "checkbox" || type == "radio") {
          if (new_value == "1") { extras.SetAttr("checked", "checked"); }
          else { extras.RemoveAttr("checked"); }
        }
        if (state != Widget::ACTIVE) { return; }
        MAIN_THREAD_EM_ASM({
          const control = document.getElementById(UTF8ToString($0));
          if (!control) return;
          const value = UTF8ToString($1);
          if (control.type === 'checkbox' || control.type === 'radio') control.checked = value === '1';
          else if (control.value !== value) control.value = value;
        }, id.c_str(), value.c_str());
      }

      void ApplyValidation(bool report) {
        if (!validation_callback_id) { return; }
        MAIN_THREAD_EM_ASM({
          const control = document.getElementById(UTF8ToString($0));
          if (!control) return;
          const error = UTF8ToString($1);
          control.setCustomValidity(error);
          if (error) control.setAttribute('aria-invalid', 'true');
          else control.removeAttribute('aria-invalid');
          if (!control.empClearValidity) {
            control.empClearValidity = () => emp.Callback($2);
            control.addEventListener('input', control.empClearValidity);
          }
          if ($3 && error) control.reportValidity();
        }, id.c_str(), validation_error.c_str(), validation_callback_id, report);
      }

      void SetValidationError(const std::string & error, bool report) {
        validation_error = error;
        if (!validation_callback_id && !error.empty()) {
          validation_callback_id = JSWrap(std::function<void()>{
            [this]() { SetValidationError("", false); }
          });
        }
        if (state == Widget::ACTIVE) { ApplyValidation(report); }
      }

      void UpdateCallback(const std::function<void(std::string)> & in_cb) { callback = in_cb; }

      void UpdateChecker(const std::function<bool(std::string)> & in_ck) { checker = in_ck; }

      void UpdateLabel(const std::string & in_label) {
        label = in_label;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateType(const std::string & in_type) {
        type = in_type;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateMin(const std::string & in_min) {
        min = in_min;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateMin(const double & in_min) {
        min = to_string(in_min);
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateMax(const std::string & in_max) {
        max = in_max;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateMax(const double & in_max) {
        max = to_string(in_max);
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateValue(const std::string & in_value) {
        Widget keep_alive(this);
        value = in_value;
        DoChange(in_value);
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateValue(const double & in_value) {
        Widget keep_alive(this);
        value = to_string(in_value);
        DoChange(value);
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateStep(const std::string & in_step) {
        step = in_step;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateStep(const double & in_step) {
        step = to_string(in_step);
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateDisabled(bool in_dis) {
        if (in_dis) {
          extras.SetAttr("disabled", "true");
        } else {
          extras.RemoveAttr("disabled");
        }
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

    public:
      virtual std::string GetType() override { return "web::InputInfo"; }
    };  // End of InputInfo definition
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Get a properly cast version of info.
    InputInfo * Info() { return (InputInfo *) info; }

    const InputInfo * Info() const { return (InputInfo *) info; }

    Input(InputInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    /// Create a new Input.
    /// @param in_cb The function to call when the Input is changed.
    /// @param in_type The type of this input.
    /// @param in_label The label that should appear on the Input.
    /// @param in_id The HTML ID to use for this Input (leave blank for auto-generated)
    /// @param show_value Do you want to add an output element to display to value of this input?
    /// @param is_checked Should the is_checked attribute by set to true or false?
    /// @param css_class Optional whitespace-separated CSS classes for the input.
    Input(const std::function<void(std::string)> & in_cb,
          const std::string & in_type,
          const std::string & in_label,
          const std::string & in_id = "",
          bool show_value           = false,
          bool is_checked           = false,
          const std::string & css_class = "")
      : WidgetFacet(in_id) {
      info = new InputInfo(in_id);
      if (!css_class.empty()) { AddClass(css_class); }

      Info()->label      = in_label;
      Info()->type       = in_type;
      Info()->show_value = show_value;
      Info()->autofocus  = false;
      Info()->curr_val   = "";

      Info()->callback      = in_cb;
      InputInfo * b_info    = Info();
      Info()->callback_id   = JSWrap(std::function<void(std::string)>(
        [b_info](std::string new_val) { b_info->DoChange(new_val); }));
      Info()->onchange_info = emp::to_string(
        "emp.Callback(",
        Info()->callback_id,
        ", ['checkbox', 'radio'].includes(this.type) ? (this.checked ? '1' : '0') : this.value);");
      // Allows user to set the checkbox to start out on/checked
      if (in_type.compare("checkbox") == 0 && is_checked) { this->SetAttr("checked", "true"); }
    }

    /// Link to an existing Input.
    Input(const Input & in) : WidgetFacet(in) { ; }

    Input(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsInput()); }

    Input() : WidgetFacet("") { info = nullptr; }

    virtual ~Input() { ; }

    using INFO_TYPE = InputInfo;

    /// Set a new callback function to trigger when the Input is clicked.
    Input & Callback(const std::function<void(std::string)> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }

    /// Set a new checker function to trigger when the Input is clicked.
    Input & Checker(const std::function<bool(std::string)> & in_ck) {
      Info()->UpdateChecker(in_ck);
      return *this;
    }

    /// Return an empty string for success, otherwise a message explaining the rejection.
    Input & Validator(const std::function<std::string(const std::string &)> & fn) {
      Info()->validator = fn;
      return *this;
    }

    /// Synchronize the model and live control without validation, callbacks, or replacing DOM.
    Input & SetValueSilently(const std::string & value) {
      Info()->UpdateValueSilently(value);
      return *this;
    }

    Input & SetValueSilently(double value) { return SetValueSilently(emp::to_string(value)); }

    /// A non-owning setter for linked controls; becomes a no-op when this input is destroyed.
    std::function<void(const std::string &)> GetSilentValueSetter(
      std::function<std::string(const std::string &)> describe = {}) {
      return [target = Info(), callback_id = Info()->callback_id,
              describe = std::move(describe)](const std::string & value) {
        if (!emp::IsJSCallbackActive(callback_id)) { return; }
        Widget keep_alive(target);
        target->UpdateValueSilently(value);
        if (describe) {
          const auto text = describe(value);
          target->extras.SetAttr("aria-valuetext", text);
          if (target->state == Widget::ACTIVE) {
            Attributes::Apply(target->id, "aria-valuetext", text);
          }
        }
      };
    }

    Input & SetValidationError(const std::string & message, bool report = true) {
      Info()->SetValidationError(message, report);
      return *this;
    }

    Input & ClearValidationError() { return SetValidationError("", false); }

    const std::string & GetValidationError() const { return Info()->validation_error; }

    Input & RejectValue(const std::string & previous, const std::string & message) {
      return SetValueSilently(previous).SetValidationError(message);
    }

    /// Set a new label to appear on this Input.
    Input & Label(const std::string & in_label) {
      Info()->UpdateLabel(in_label);
      return *this;
    }

    /// Update the type
    Input & Type(const std::string & in_t) {
      Info()->UpdateType(in_t);
      return *this;
    }

    /// Update the min
    Input & Min(const std::string & in_m) {
      Info()->UpdateMin(in_m);
      return *this;
    }

    /// Update the min
    Input & Min(const double & in_m) {
      Info()->UpdateMin(in_m);
      return *this;
    }

    /// Update the max
    Input & Max(const std::string & in_m) {
      Info()->UpdateMax(in_m);
      return *this;
    }

    /// Update the max
    Input & Max(const double & in_m) {
      Info()->UpdateMax(in_m);
      return *this;
    }

    /// Update the current value
    Input & Value(const std::string & in_m) {
      Info()->UpdateValue(in_m);
      return *this;
    }

    /// Update the current value
    Input & Value(const double & in_m) {
      Info()->UpdateValue(in_m);
      return *this;
    }

    /// Update the current step size
    Input & Step(const std::string & in_m) {
      Info()->UpdateStep(in_m);
      return *this;
    }

    /// Update the current step size
    Input & Step(const double & in_m) {
      Info()->UpdateStep(in_m);
      return *this;
    }

    /// Setup this Input to have autofocus (or remove it!)
    Input & Autofocus(bool in_af = true) {
      Info()->UpdateAutofocus(in_af);
      return *this;
    }

    /// Setup this Input to be disabled (or re-enable it!)
    Input & Disabled(bool in_dis = true) {
      Info()->UpdateDisabled(in_dis);
      return *this;
    }

    /// Get the current label on this Input.
    const std::string & GetCurrValue() const { return Info()->curr_val; }

    /// Get the current label on this Input.
    const std::string & GetLabel() const { return Info()->label; }

    /// Get the current type of this input.
    const std::string & GetType() const { return Info()->type; }

    /// Get the current min of this input.
    const std::string & GetMin() const { return Info()->min; }

    /// Get the current max of this input.
    const std::string & GetMax() const { return Info()->max; }

    /// Get the value attribute of this input.
    const std::string & GetValue() const { return Info()->value; }

    /// Get the value attribute of this input.
    const std::string & GetStep() const { return Info()->step; }

    /// Determine if this Input currently has autofocus.
    bool HasAutofocus() const { return Info()->autofocus; }

    /// Determine if this Input is currently disabled.
    bool IsDisabled() const { return Info()->extras.HasAttr("disabled"); }
  };


}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_INPUT_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: evt imdr chriscoyier codepen
