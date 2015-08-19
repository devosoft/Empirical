#ifndef EMP_WEB_TEXT_AREA_H
#define EMP_WEB_TEXT_AREA_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the TextArea widget.
//

#include "Widget.h"

namespace emp {
namespace web {

  class TextArea : public internal::WidgetFacet<TextArea> {
    friend class TextAreaInfo;
  protected:

    // TextAreas associated with the same DOM element share a single TextAreaInfo object.
    class TextAreaInfo : public internal::WidgetInfo {
      friend TextArea;
    protected:
      int cols;                 // How many columns of text in the area?
      int rows;                 // How many rows of text in the area?
      int max_length;           // Maximum number of total characters allowed.

      std::string placeholder;  // Info text prior to initial typing.

      bool autofocus;
      bool disabled;
      
      std::function<void()> callback;
      uint32_t callback_id;
      std::string onclick_info;
      
      TextAreaInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      TextAreaInfo(const TextAreaInfo &) = delete;               // No copies of INFO allowed
      TextAreaInfo & operator=(const TextAreaInfo &) = delete;   // No copies of INFO allowed
      virtual ~TextAreaInfo() {
        if (callback_id) emp::JSDelete(callback_id);         // Delete callback wrapper.
      }
      
      virtual bool IsTextAreaInfo() const override { return true; }

      void DoCallback() {
        callback();
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML << "<textarea ";                                   // Start the textarea tag.
        if (title != "") HTML << " title=\"" << title << "\"";  // Add a title if there is one.
        if (disabled) { HTML << " disabled=true"; }             // Check if should be disabled
        HTML << " id=\"" << id << "\"";                         // Indicate ID.
        HTML << " onclick=\"" << onclick_info << "\"";          // Indicate action on click.
        HTML << ">" << label << "</textarea>";                  // Close and label the textarea
      }
      
      void UpdateCallback(const std::function<void()> & in_cb) {
        if (callback_id) emp::JSDelete(callback_id);    // Delete previous callback wrapper.
        callback_id = JSWrap(in_cb);                    // Save id for callback trigger.
        onclick_info = std::string("emp.Callback(") + std::to_string(callback_id) + ")";
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }
      void UpdateCallback(const std::string in_cb_info) {
        if (callback_id) emp::JSDelete(callback_id);    // Delete previous callback wrapper.
        callback_id = 0;                                // No ID currently in callback.
        onclick_info = in_cb_info;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }

      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }
      void UpdateDisabled(bool in_dis) {
        disabled = in_dis;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }
      
    public:
      virtual std::string GetType() override { return "web::TextAreaInfo"; }
    }; // End of TextAreaInfo definition
    
    
    // Get a properly cast version of indo.  
    TextAreaInfo * Info() { return (TextAreaInfo *) info; }
    const TextAreaInfo * Info() const { return (TextAreaInfo *) info; }
 
    TextArea(TextAreaInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    TextArea(const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new TextAreaInfo(in_id);
    
      Info()->cols = 20;
      Info()->rows = 1;
      Info()->max_length = -1;
      Info()->placeholder = "";
      Info()->autofocus = false;
      Info()->disabled = false;
      
      Info()->callback = in_cb;
      TextAreaInfo * ta_info = Info();
      Info()->callback_id = JSWrap( std::function<void()>( [ta_info](){ta_info->DoCallback();} )  );
      Info()->onclick_info = emp::to_string("emp.Callback(", Info()->callback_id, ")");
    }
    TextArea(const TextArea & in) : WidgetFacet(in) { ; }
    TextArea(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsTextAreaInfo()); }
    virtual ~TextArea() { ; }

    using INFO_TYPE = TextAreaInfo;

    TextArea & Callback(const std::function<void()> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }
    TextArea & Callback(const std::string in_cb_info) {
      Info()->UpdateCallback(in_cb_info);
      return *this;
    }
    TextArea & Autofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }
    TextArea & Disabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }
    
    const std::string & GetLabel() const { return Info()->label; }
    const std::string & GetTitle() const { return Info()->title; }
    bool HasAutofocus() const { return Info()->autofocus; }
    bool IsDisabled() const { return Info()->disabled; }
  };


}
}

#endif
