#ifndef EMP_UI_SELECTOR_H
#define EMP_UI_SELECTOR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Selector widget.
//

#include "../web/JSWrap.h"
#include "../tools/vector.h"

#include "Widget.h"

namespace emp {
namespace UI {

  class Selector : public internal::Widget<Selector> {
  protected:
    emp::vector<std::string> options;               // What are the options to choose from?
    emp::vector<std::function<void()> > callbacks;  // Which funtion to run for each option?
    int select_id;                                  // Which index is currently selected?

    bool autofocus;
    bool disabled;
    
    uint32_t callback_id;
      
    void WriteHTML(std::ostream & os) {
      os << "<select";                                      // Start the select tag.
      if (disabled) { os << " disabled=true"; }             // Check if should be disabled
      os << " id=\"" << div_id << obj_ext << "\"";          // Indicate ID.

      // Indicate action on change.
      os << " onchange=\"emp.Callback(" << callback_id << ", this.selectedIndex)\">";

      // List out options
      for (int i = 0; i < (int) options.size(); i++) {
        os << "<option value=\"" << i;
        if (i == select_id) os << " selected";
        os << "\">" << options[i] << "</option>";
      }
      os << "</select>";
    }

    void DoChange(int new_id) {
      emp::Alert("Changing to ", new_id);
      select_id = new_id;
      if (callbacks[new_id]) callbacks[new_id]();
    }
      
  public:
    Selector(const std::string & in_name="")
      : Widget(in_name)
      , select_id(0)
      , autofocus(false), disabled(false)
    {
      callback_id = JSWrap( std::function<void(int)>([this](int new_id){DoChange(new_id);}) );
      obj_ext = "__s";
    }

    Selector(const Selector & _in)
      : Widget(_in)
      , options(_in.options), callbacks(_in.callbacks)
      , select_id(_in.select_id)
      , autofocus(_in.autofocus), disabled(_in.disabled)
    {
      callback_id = JSWrap( std::function<void(int)>([this](int new_id){DoChange(new_id);}) );
      obj_ext = "__s";      
    }

    ~Selector() {
      // @CAO Can't delete unless we're sure no other copies of Selector are using id...
      //emp::JSDelete(callback_id);  // Delete callback wrapper.
    }

    int GetSelectID() const { return select_id; }
    int GetNumOptions() const { return (int) options.size(); }
    const std::string & GetOption(int id) const { return options[id]; }

    Selector & SetOption(const std::string & in_option,
                         const std::function<void()> & in_cb,
                         int opt_id=-1) {
      // If no option id was specified, choose the next one.
      if (opt_id < 0) opt_id = (int) options.size();

      // If we need more room for options, increase the array size.
      if (opt_id >= (int) options.size()) {
        options.resize(opt_id+1);
        callbacks.resize(opt_id+1);
      }
      options[opt_id] = in_option;
      callbacks[opt_id] = in_cb;
      return *this;
    }
    
    Selector & SetOption(const std::string & in_option, int opt_id=-1) {
      return SetOption(in_option, std::function<void()>([](){}), opt_id);
    }

    Selector & Autofocus(bool in_af) { autofocus = in_af; return *this; }
    Selector & Disabled(bool in_dis) { disabled = in_dis; return *this; }
    
    bool IsDisabled() const { return disabled; }
    
    static std::string TypeName() { return "Selector"; }
  };


};
};

#endif
