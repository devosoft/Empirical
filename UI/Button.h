#ifndef EMP_UI_BUTTON_H
#define EMP_UI_BUTTON_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Button widget.
//


#include "Widget.h"

namespace emp {
namespace UI {

  class Button : public internal::Widget<Button> {
  protected:
    std::string label;
      
    bool autofocus;
    bool disabled;
    std::string title;
    
    uint32_t callback_id;
    std::string onclick_info;
      
    void WriteHTML(std::ostream & os) {
      os << "<button";                                      // Start the button tag.
      if (title != "") os << " title=\"" << title << "\"";  // Add a title if there is one.
      if (disabled) { os << " disabled=true"; }             // Check if should be disabled
      os << " id=\"" << div_id << obj_ext << "\"";          // Indicate ID.

      // Indicate action on click.
      os << " onclick=\"" << onclick_info << "\"";
      os << ">" << label << "</button>";       // Close and label the button.
    }
      
  public:
    Button(const std::string & in_cb_info, const std::string & in_label,
           const std::string & in_name="")
      : Widget(in_name)
      , label(in_label)
      , autofocus(false), disabled(false), title("")
      , callback_id(0)
      , onclick_info(in_cb_info)
    {
      obj_ext = "__b";
    }
    Button(const std::function<void()> & in_cb, const std::string & in_label,
           const std::string & in_name="")
      : Button(std::string(""), in_label, in_name)
    {
      obj_ext = "__b";
      callback_id = JSWrap(in_cb);
      onclick_info = std::string("emp.Callback(") + std::to_string(callback_id) + ")";
    }
    ~Button() {
      // @CAO Can't delete unless we're sure no other copies of Button are using id...
      //emp::JSDelete(callback_id);  // Delete callback wrapper.
    }

    Button & Callback(const std::function<void()> & in_cb) {
      if (callback_id) emp::JSDelete(callback_id);    // Delete previous callback wrapper.
      callback_id = JSWrap(in_cb);   // Save id for callback trigger.
      onclick_info = std::string("emp.Callback(") + std::to_string(callback_id) + ")";
      return *this;
    }
    Button & Callback(const std::string in_cb_info) {
      if (callback_id) emp::JSDelete(callback_id);  // Delete previous callback wrapper.
      callback_id = 0;   // No ID currently in callback.
      onclick_info = in_cb_info;
      return *this;
    }
    Button & Label(const std::string & in_label) { label = in_label; return *this; }
    
    Button & Autofocus(bool in_af) { autofocus = in_af; return *this; }
    Button & Disabled(bool in_dis) { disabled = in_dis; return *this; }
    Button & Title(const std::string & t) { title = t; return *this; }
    
    bool IsDisabled() const { return disabled; }
    
    static std::string TypeName() { return "Button"; }
  };


};
};

#endif
