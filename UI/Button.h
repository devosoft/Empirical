#ifndef EMP_UI_BUTTON_H
#define EMP_UI_BUTTON_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Button widget.
//


#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Button_detail class
  namespace internal { class Button_detail; };

  // Specify the Button class for use in return values in the Button_detail definitions below.
  using Button = internal::Widget_wrap<internal::Button_detail,
                                       const std::function<void()> &,
                                       const std::string &>;  
  
  // Define Button_detail in internal namespace (i.e., details should not be accessed directly.)
  namespace internal {
    class Button_detail : public Widget_base {
    protected:
      std::function<void()> callback;
      std::string label;
      
      bool autofocus;
      bool disabled;
      std::string title;

      uint32_t callback_id;
      
      void WriteHTML(std::ostream & os) {
        os << "<button";                          // Start the button tag.
        if (title != "") {                          // If the button has a title, add it!
          os << " title=\"" << title << "\"";
        }

        if (disabled) { os << " disabled=true"; }

        // Indicate ID.
        os << " id=\"" << div_id << obj_ext << "\"";

        // Indicate action on click.
        os << " onclick=\"empCppCallback(" << std::to_string(callback_id) << ")\"";
        
        os << ">" << label << "</button>";       // Close and label the button.
      }
    public:
      Button_detail(const std::function<void()> & in_cb, const std::string & in_label)
        : callback(in_cb), label(in_label)
        , autofocus(false), disabled(false), title("")
        , callback_id(JSWrap(callback))
      { obj_ext = "__b"; }
      ~Button_detail() {
        // @CAO Can't delete unless we're sure no other copies of Button_detail are using id...
        //emp::JSDelete(callback_id);  // Delete callback wrapper.
      }

      Button & Callback(const std::function<void()> & in_cb) {
        emp::JSDelete(callback_id);       // Delete previous callback wrapper.
        callback = in_cb;                 // Save target function
        callback_id = JSWrap(callback);   // Save id for callback trigger.
        return (Button &) *this;
      }
      Button & Label(const std::string & in_label) { label = in_label; return (Button &) *this; }

      Button & Autofocus(bool in_af) { autofocus = in_af; return (Button &) *this; }
      Button & Disabled(bool in_dis) { disabled = in_dis; return (Button &) *this; }
      Button & Title(const std::string & t) { title = t; return (Button &) *this; }

      bool IsDisabled() const { return disabled; }
    };

  };

};
};

#endif
