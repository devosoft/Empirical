#ifndef EMP_UI_SELECTOR_H
#define EMP_UI_SELECTOR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Selector widget.
//

#include "../tools/vector.h"

#include "Widget.h"

namespace emp {
namespace UI {

  class Selector : public internal::Widget<Selector> {
  protected:
    emp::vector<std::string> options;

    bool autofocus;
    bool disabled;
    
    uint32_t callback_id;
    std::string onchange_info;
      
    void WriteHTML(std::ostream & os) {
      os << "<select";                                      // Start the select tag.
      if (disabled) { os << " disabled=true"; }             // Check if should be disabled
      os << " id=\"" << div_id << obj_ext << "\"";          // Indicate ID.

      // Indicate action on change.
      os << " onchange=\"" << onchange_info << "\">";

      // List out options
      for (int i = 0; i < (int) options.size(); i++) {
        os << "<option value=\"" << i << "\">" << options[i] << "</option>";
      }
      os << "</select>";
    }
      
  public:
    Selector(const std::string & in_cb_info, const emp::vector<std::string> & in_options,
             const std::string & in_name="")
      : Widget(in_name)
      , options(in_options)
      , autofocus(false), disabled(false)
      , callback_id(0)
      , onchange_info(in_cb_info)
    {
      obj_ext = "__s";
    }
    Selector(const std::function<void(int)> & in_cb, const emp::vector<std::string> & in_options,
           const std::string & in_name="")
      : Selector(std::string(""), in_options, in_name)
    {
      obj_ext = "__s";
      callback_id = JSWrap(in_cb);
      onchange_info = std::string("emp.Callback(")
        + std::to_string(callback_id)
        + ", this.selectedIndex)";
    }
    ~Selector() {
      // @CAO Can't delete unless we're sure no other copies of Selector are using id...
      //emp::JSDelete(callback_id);  // Delete callback wrapper.
    }

    Selector & Callback(const std::function<void(int)> & in_cb) {
      if (callback_id) emp::JSDelete(callback_id);    // Delete previous callback wrapper.
      callback_id = JSWrap(in_cb);   // Save id for callback trigger.
      onchange_info = std::string("emp.Callback(")
        + std::to_string(callback_id)
        + ", this.selectedIndex)";
      return *this;
    }
    Selector & Callback(const std::string in_cb_info) {
      if (callback_id) emp::JSDelete(callback_id);  // Delete previous callback wrapper.
      callback_id = 0;   // No ID currently in callback.
      onchange_info = in_cb_info;
      return *this;
    }
    
    Selector & SetOption(const std::string & in_option, int opt_id=-1) {
      // If no option id was specified, choose the next one.
      if (opt_id < 0) opt_id = (int) options.size();

      // If we need more room for options, increase the array size.
      if (opt_id >= (int) options.size()) options.resize(opt_id+1);
      options[opt_id] = in_option;
      return *this;
    }
    
    Selector & Autofocus(bool in_af) { autofocus = in_af; return *this; }
    Selector & Disabled(bool in_dis) { disabled = in_dis; return *this; }
    
    bool IsDisabled() const { return disabled; }
    
    static std::string TypeName() { return "Selector"; }
  };


};
};

#endif
