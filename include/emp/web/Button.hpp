/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018.
 *
 *  @file  Button.hpp
 *  @brief Create/control an HTML button and call a specified function when that button is clicked.
 *
 *  Use example:
 *
 *    emp::web::Button my_button(MyFun, "Button Name", "html_id");
 *
 *  Where my_button is the C++ object linking to the button, MyFun is the
 *  function you want to call on clicks, "Button Name" is the label on the
 *  button itself, and "html_id" is the optional id you want it to use in the
 *  HTML code (otherwise it will generate a unique name on its own.)
 *
 *  Member functions to set state:
 *    Button & SetCallback(const std::function<void()> & in_callback)
 *    Button & SetLabel(const std::string & in_label)
 *    Button & SetAutofocus(bool in_af)
 *    Button & SetDisabled(bool in_dis)
 *
 *  Retriving current state:
 *    const std::string & GetLabel() const
 *    bool HasAutofocus() const
 *    bool IsDisabled() const
 */

#ifndef EMP_WEB_BUTTON_H
#define EMP_WEB_BUTTON_H

#include "init.hpp"
#include "Widget.hpp"

namespace emp {
namespace web {

  /// Create or control an HTML Button object that you can manipulate and update as needed.
  class Button : public internal::WidgetFacet<Button> {
    friend class ButtonInfo;
  protected:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    // Buttons associated with the same DOM element share a single ButtonInfo object.
    class ButtonInfo : public internal::WidgetInfo {
      friend Button;
    protected:
      std::string label;
      std::function<void()> callback;
      uint32_t callback_id;

      ButtonInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      ButtonInfo(const ButtonInfo &) = delete;               // No copies of INFO allowed
      ButtonInfo & operator=(const ButtonInfo &) = delete;   // No copies of INFO allowed
      virtual ~ButtonInfo() {
        if (callback_id) emp::JSDelete(callback_id);         // Delete callback wrapper.
      }

      std::string GetTypeName() const override { return "ButtonInfo"; }

      void DoCallback() {
        callback();
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str(to_string("<button id=\"", id, "\">", label, "</button>"));
      }

      void UpdateCallback(const std::function<void()> & in_cb) {
        callback = in_cb;
      }

      void UpdateLabel(const std::string & in_label) {
        label = in_label;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }

    public:
      virtual std::string GetType() override { return "web::ButtonInfo"; }
    }; // End of ButtonInfo definition
    #endif // DOXYGEN_SHOULD_SKIP_THIS


    // Get a properly cast version of indo.
    ButtonInfo * Info() { return (ButtonInfo *) info; }
    const ButtonInfo * Info() const { return (ButtonInfo *) info; }

    Button(ButtonInfo * in_info) : WidgetFacet(in_info) { ; }

  public:

    /// Create a new button.
    /// @param in_cb The function to call when the button is clicked.
    /// @param in_label The label that should appear on the button.
    /// @param in_id The HTML ID to use for this button (leave blank for auto-generated)
    Button(const std::function<void()> & in_cb, const std::string & in_label,
           const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new ButtonInfo(in_id);

      Info()->label = in_label;

      Info()->callback = in_cb;
      ButtonInfo * b_info = Info();
      Info()->callback_id = JSWrap( std::function<void()>( [b_info](){b_info->DoCallback();} )  );
      SetAttr("onclick", emp::to_string("emp.Callback(", Info()->callback_id, ")"));
    }

    /// Link to an existing button.
    Button(const Button & in) : WidgetFacet(in) { ; }
    Button(const Widget & in) : WidgetFacet(in) { emp_assert(in.IsButton()); }
    Button() : WidgetFacet("") { info = nullptr; }
    virtual ~Button() { ; }

    using INFO_TYPE = ButtonInfo;

    /// Set a new callback function to trigger when the button is clicked.
    Button & SetCallback(const std::function<void()> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }

    /// Set a new label to appear on this Button.
    Button & SetLabel(const std::string & in_label) { Info()->UpdateLabel(in_label); return *this; }

    /// Setup this button to have autofocus (or remove it!)
    Button & SetAutofocus(bool _in=true) { SetAttr("autofocus", ToJSLiteral(_in)); return *this; }

    /// Setup this button to be disabled (or re-enable it!)
    Button & SetDisabled(bool _in=true) {
      if (_in) SetAttr("disabled", "disabled");
      else {
        Info()->extras.RemoveAttr("disabled");
        if (IsActive()) Info()->ReplaceHTML();
      }

      return *this;
    }

    /// Get the current label on this button.
    const std::string & GetLabel() const { return Info()->label; }

    /// Determine if this button currently has autofocus.
    bool HasAutofocus() const { return GetAttr("autofocus") == "true"; }

    /// Determine if this button is currently disabled.
    bool IsDisabled() const { return Info()->extras.HasAttr("disabled"); }
  };


}
}

#endif
