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
    std::function<void()> callback;
    std::string label;
      
    bool autofocus;
    bool disabled;
    std::string title;
    
    uint32_t callback_id;
      
    void WriteHTML(std::ostream & os) {
      os << "<button";                                      // Start the button tag.
      if (title != "") os << " title=\"" << title << "\"";  // Add a title if there is one.
      if (disabled) { os << " disabled=true"; }             // Check if should be disabled
      os << " id=\"" << div_id << obj_ext << "\"";          // Indicate ID.

      // Indicate action on click.
      os << " onclick=\"empCppCallback(" << std::to_string(callback_id) << ")\"";
      
      os << ">" << label << "</button>";       // Close and label the button.
    }
  public:
    Button(const std::function<void()> & in_cb, const std::string & in_label,
           const std::string & in_name="")
      : Widget(in_name)
      , callback(in_cb), label(in_label)
      , autofocus(false), disabled(false), title("")
      , callback_id(JSWrap(callback))
    { obj_ext = "__b"; }
    ~Button() {
      // @CAO Can't delete unless we're sure no other copies of Button are using id...
      //emp::JSDelete(callback_id);  // Delete callback wrapper.
    }

    Button & Callback(const std::function<void()> & in_cb) {
      emp::JSDelete(callback_id);       // Delete previous callback wrapper.
      callback = in_cb;                 // Save target function
      callback_id = JSWrap(callback);   // Save id for callback trigger.
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
