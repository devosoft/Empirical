/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief Create/control an HTML checkbox and call a specified function when it changes.
 *
 *  Use example:
 *
 *    emp::web::CheckBox my_checkbox(MyFun, "html_id");
 *
 *  Where:
 *   - my_checkbox is the C++ object linking to the checkbox
 *   - MyFun takes a bool argument and is called every time the checkbox status changes
 *   - "html_id" is the optional id you want it to use in the HTML code
 *     (otherwise it will generate a unique name on its own.)
 *
 *  Member functions to set state:
 *    CheckBox & SetCallback(const std::function<void(bool)> & in_callback)
 *    CheckBox & SetAutofocus(bool in_af)
 *    CheckBox & SetDisabled(bool in_dis)
 *
 *  Retrieving current state:
 *    bool IsSet() const
 *    bool HasAutofocus() const
 *    bool IsDisabled() const
 */

#ifndef EMP_WEB_CHECKBOX_HPP_INCLUDE
#define EMP_WEB_CHECKBOX_HPP_INCLUDE

#include <cstdint>

#include "init.hpp"
#include "Widget.hpp"

namespace emp {
namespace web {

  /// Create or control an HTML CheckBox object that you can manipulate and update as needed.
  class CheckBox : public internal::WidgetFacet<CheckBox> {
    friend class CheckBoxInfo;
  protected:
    using cb_type = std::function<void(bool)>; 

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    // CheckBoxes associated with the same DOM element share a single CheckBoxInfo object.
    class CheckBoxInfo : public internal::WidgetInfo {
      friend CheckBox;
    protected:
      cb_type callback;
      uint32_t callback_id;

      CheckBoxInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      CheckBoxInfo(const CheckBoxInfo &) = delete;               // No copies of INFO allowed
      CheckBoxInfo & operator=(const CheckBoxInfo &) = delete;   // No copies of INFO allowed
      virtual ~CheckBoxInfo() {
        if (callback_id) emp::JSDelete(callback_id);         // Delete callback wrapper.
      }

      std::string GetTypeName() const override { return "CheckBoxInfo"; }

      void DoCallback(bool new_value) {
        callback(new_value);
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str(to_string("<input type=\"checkbox\" id=\"", id, "\">"));
      }

      void UpdateCallback(const cb_type & in_cb) {
        callback = in_cb;
      }

    public:
      virtual std::string GetType() override { return "web::CheckBoxInfo"; }
    }; // End of CheckBoxInfo definition
    #endif // DOXYGEN_SHOULD_SKIP_THIS


    // Get a properly cast version of info.
    CheckBoxInfo * Info() { return (CheckBoxInfo *) info; }
    const CheckBoxInfo * Info() const { return (CheckBoxInfo *) info; }

    CheckBox(CheckBoxInfo * in_info) : WidgetFacet(in_info) { ; }

  public:

    /// Create a new checkbox.
    /// @param in_cb The function to call when the checkbox is toggled.
    /// @param in_id The HTML ID to use for this checkbox (leave blank for auto-generated)
    CheckBox(const cb_type & in_cb, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new CheckBoxInfo(in_id);

      Info()->callback = in_cb;
      CheckBoxInfo * b_info = Info();
      Info()->callback_id = JSWrap( (cb_type) [b_info](bool in){b_info->DoCallback(in);} );
      SetAttr("onclick", emp::to_string("emp.Callback(", Info()->callback_id, ", this.checked)"));
    }

    /// Link to an existing checkbox.
    CheckBox(const CheckBox & in) : WidgetFacet(in) { ; }
    CheckBox(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsCheckBox()); }
    CheckBox() : WidgetFacet("") { info = nullptr; }
    virtual ~CheckBox() { ; }

    using INFO_TYPE = CheckBoxInfo;

    bool IsChecked() const { return GetAttr("checked") == "1"; }
    CheckBox & SetChecked(bool in=true) { SetAttr("checked", in); return *this; }

    /// Set a new callback function to trigger when the checkbox is toggled.
    CheckBox & SetCallback(const cb_type & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }

    /// Setup this checkbox to have autofocus (or remove it!)
    CheckBox & SetAutofocus(bool _in=true) { SetAttr("autofocus", ToJSLiteral(_in)); return *this; }

    /// Setup this checkbox to be disabled (or re-enable it!)
    CheckBox & SetDisabled(bool _in=true) {
      if (_in) SetAttr("disabled", "disabled");
      else {
        Info()->extras.RemoveAttr("disabled");
        if (IsActive()) Info()->ReplaceHTML();
      }

      return *this;
    }

    /// Determine if this checkbox currently has autofocus.
    bool HasAutofocus() const { return GetAttr("autofocus") == "true"; }

    /// Determine if this checkbox is currently disabled.
    bool IsDisabled() const { return Info()->extras.HasAttr("disabled"); }
  };


}
}

#endif // #ifndef EMP_WEB_CHECKBOX_HPP_INCLUDE
