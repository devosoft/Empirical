//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Specs for the Selector widget.
//
//  A Selector widget provides the user with a pull-down menu.  It can be
//  examined at any time (via GetSelectID()) or else alerts call a designated
//  function when a particular option is chosen.
//
//     UI::Selector sel("sel");
//
//     sel.SetOption("Option 1");
//     sel.SetOption("Option B", TriggerB) ;
//     sel.SetOption("Option the Third", [](){ emp::Alert("3 chosen!"} );
//     sel.SetOption("Option IV");
//
//  In this example, the second option will call TriggerB when it is chosen,
//  while the third option will call the provided lambda function.

#ifndef EMP_WEB_SELECTOR_H
#define EMP_WEB_SELECTOR_H


#include "../tools/vector.h"
#include "JSWrap.h"

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
      size_t select_id;                               // Which index is currently selected?

      bool autofocus;
      bool disabled;

      size_t callback_id;

      SelectorInfo(const std::string & in_id="")
        : internal::WidgetInfo(in_id), select_id(0), autofocus(false), disabled(false) { ; }
      SelectorInfo(const SelectorInfo &) = delete;               // No copies of INFO allowed
      SelectorInfo & operator=(const SelectorInfo &) = delete;   // No copies of INFO allowed
      virtual ~SelectorInfo() {
        if (callback_id) emp::JSDelete(callback_id);             // Delete callback wrapper.
      }

      std::string TypeName() const override { return "SelectorInfo"; }
      virtual bool IsSelectorInfo() const override { return true; }

      void SetOption(const std::string & name, const std::function<void()> & cb, size_t id) {
        // If we need more room for options, increase the array size.
        if (id >= options.size()) {
          options.resize(id+1);
          callbacks.resize(id+1);
        }
        options[id] = name;
        callbacks[id] = cb;
      }
      void SetOption(const std::string & name, const std::function<void()> & cb) {
        SetOption(name, cb, options.size()); // No option id specified, so choose the next one.
      }

      void DoChange(size_t new_id) {
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
        for (size_t i = 0; i < options.size(); i++) {
          HTML << "<option value=\"" << i << "\"";
          if (i == select_id) HTML << " selected";
          HTML << ">" << options[i] << "</option>";
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
        JSWrap( std::function<void(size_t)>([s_info](size_t new_id){s_info->DoChange(new_id);}) );
    }
    Selector(const Selector & in) : WidgetFacet(in) { ; }
    Selector(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsSelectorInfo()); }
    virtual ~Selector() { ; }

    using INFO_TYPE = SelectorInfo;


    size_t GetSelectID() const { return Info()->select_id; }
    size_t GetNumOptions() const { return Info()->options.size(); }
    const std::string & GetOption(size_t id) const { return Info()->options[id]; }
    bool HasAutofocus() const { return Info()->autofocus; }
    bool IsDisabled() const { return Info()->disabled; }

    Selector & SelectID(size_t id) { Info()->select_id = id; return *this; }

    Selector & SetOption(const std::string & in_option,
                         const std::function<void()> & in_cb) {
      Info()->SetOption(in_option, in_cb);
      return *this;
    }
    Selector & SetOption(const std::string & in_option,
                         const std::function<void()> & in_cb,
                         size_t opt_id) {
      Info()->SetOption(in_option, in_cb, opt_id);
      return *this;
    }

    Selector & SetOption(const std::string & in_option) {
      return SetOption(in_option, std::function<void()>([](){}));
    }
    Selector & SetOption(const std::string & in_option, size_t opt_id) {
      return SetOption(in_option, std::function<void()>([](){}), opt_id);
    }

    Selector & Autofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }
    Selector & Disabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }
  };


}
}

#endif
