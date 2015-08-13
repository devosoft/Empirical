#ifndef EMP_WEB_SELECTOR_H
#define EMP_WEB_SELECTOR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Selector widget.
//

#include "../emtools/JSWrap.h"
#include "../tools/vector.h"

#include "Widget.h"

namespace emp {
namespace web {

  class Selector : public internal::WidgetFacet<Selector> {
    friend class SelectorInfo;
  protected:

    class SelectorInfo : public internal::WidgetInfo {
      friend Selector;
    protected:
      emp::vector<std::string> options;               // What are the options to choose from?
      emp::vector<std::function<void()> > callbacks;  // Which funtion to run for each option?
      int select_id;                                  // Which index is currently selected?
      
      bool autofocus;
      bool disabled;
    
      uint32_t callback_id;
      
      SelectorInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      SelectorInfo(const SelectorInfo &) = delete;               // No copies of INFO allowed
      SelectorInfo & operator=(const SelectorInfo &) = delete;   // No copies of INFO allowed
      virtual ~SelectorInfo() {
        if (callback_id) emp::JSDelete(callback_id);             // Delete callback wrapper.
      }

      virtual bool IsSelectorInfo() const override { return true; }

      void SetOption(const std::string & in_option,
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
      }
    
      void DoChange(int new_id) {
        // emp::Alert("Changing to ", new_id);
        select_id = new_id;
        if (callbacks[new_id]) callbacks[new_id]();
      }
      
      virtual void GetHTML(std::stringstream & HTML) override {
        HTML << "<select";                              // Start the select tag.
        if (disabled) { HTML << " disabled=true"; }     // Check if should be disabled
        HTML << " id=\"" << id << "\"";                 // Indicate ID.
        
        // Indicate action on change.
        HTML << " onchange=\"emp.Callback(" << callback_id << ", this.selectedIndex)\">";
        
        // List out options
        for (int i = 0; i < (int) options.size(); i++) {
          HTML << "<option value=\"" << i;
          if (i == select_id) HTML << " selected";
          HTML << "\">" << options[i] << "</option>";
        }
        HTML << "</select>";
      }

      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) ReplaceHTML();
      }
      void UpdateDisabled(bool in_dis) {
        disabled = in_dis;
        if (state == Widget::ACTIVE) ReplaceHTML();
      }
      
    public:
      virtual std::string GetType() override { return "web::SelectorInfo"; }
    };  // End of SelectorInfo class.


    // Get a properly cast version of indo.  
    SelectorInfo * Info() { return (SelectorInfo *) info; }
    const SelectorInfo * Info() const { return (SelectorInfo *) info; }
 
    Selector(SelectorInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    Selector(const std::string & in_id="") : WidgetFacet(in_id)
    {
      info = new SelectorInfo(in_id);
      
      Info()->select_id = 0;
      Info()->autofocus = false;
      Info()->disabled = false;

      SelectorInfo * s_info = Info();
      Info()->callback_id =
        JSWrap( std::function<void(int)>([s_info](int new_id){s_info->DoChange(new_id);}) );
    }
    Selector(const Selector & in) : WidgetFacet(in) { ; }
    Selector(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsSelectorInfo()); }
    virtual ~Selector() { ; }

    using INFO_TYPE = SelectorInfo;


    int GetSelectID() const { return Info()->select_id; }
    int GetNumOptions() const { return (int) Info()->options.size(); }
    const std::string & GetOption(int id) const { return Info()->options[id]; }
    bool HasAutofocus() const { return Info()->autofocus; }
    bool IsDisabled() const { return Info()->disabled; }
    
    Selector & SetOption(const std::string & in_option,
                         const std::function<void()> & in_cb,
                         int opt_id=-1) {
      Info()->SetOption(in_option, in_cb, opt_id);
      return *this;
    }
    
    Selector & SetOption(const std::string & in_option, int opt_id=-1) {
      return SetOption(in_option, std::function<void()>([](){}), opt_id);
    }

    Selector & Autofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }
    Selector & Disabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }
  };


}
}

#endif
