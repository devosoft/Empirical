#ifndef EMP_WEB_TEXT_AREA_H
#define EMP_WEB_TEXT_AREA_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the TextArea widget.
//
//
//  Developer notes:
//  * Callback does a lot of string-copies at the moment; should be streamlined.
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

      std::string cur_text;     // Text that should currently be in the box.

      bool autofocus;
      bool disabled;
      
      std::function<void(const std::string &)> callback;
      uint32_t callback_id;
            
      TextAreaInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      TextAreaInfo(const TextAreaInfo &) = delete;               // No copies of INFO allowed
      TextAreaInfo & operator=(const TextAreaInfo &) = delete;   // No copies of INFO allowed
      virtual ~TextAreaInfo() {
        if (callback_id) emp::JSDelete(callback_id);             // Delete callback wrapper.
      }
      
      virtual bool IsTextAreaInfo() const override { return true; }

      void DoCallback(std::string in_text) {
        cur_text = in_text;
        if (callback) callback(cur_text);
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML << "<textarea ";                                   // Start the textarea tag.
        if (disabled) { HTML << " disabled=true"; }             // Check if should be disabled
        HTML << " id=\"" << id << "\""                          // Indicate ID.
             << " onkeyup=\"emp.Callback(" << callback_id << ", $(this).val())\""
             << " rows=\"" << rows << "\""
             << " cols=\"" << cols << "\"";
        if (max_length >= 0) { HTML << " maxlength=\"" << max_length << "\""; }
        HTML << ">" << "testing" << "</textarea>";              // Close and label the textarea
      }
      
      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }

      void UpdateCallback(const std::function<void(const std::string &)> & in_cb) {
        callback = in_cb;
      }

      void UpdateDisabled(bool in_dis) {
        disabled = in_dis;
        if (state == Widget::ACTIVE) ReplaceHTML();     // If node is active, immediately redraw!
      }
      
      void UpdateText(const std::string & in_string) {
        EM_ASM_ARGS({
            var id = Pointer_stringify($0);
            var text = Pointer_stringify($1);
            $('#' + id).val(text);
          }, id.c_str(), in_string.c_str());
      }

    public:
      virtual std::string GetType() override { return "web::TextAreaInfo"; }
    }; // End of TextAreaInfo definition
    
    
    // Get a properly cast version of indo.  
    TextAreaInfo * Info() { return (TextAreaInfo *) info; }
    const TextAreaInfo * Info() const { return (TextAreaInfo *) info; }
 
    TextArea(TextAreaInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    TextArea(std::function<void(const std::string &)> in_cb, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new TextAreaInfo(in_id);
    
      Info()->cols = 20;
      Info()->rows = 1;
      Info()->max_length = -1;
      Info()->cur_text = "";
      Info()->autofocus = false;
      Info()->disabled = false;
      
      Info()->callback = in_cb;
      TextAreaInfo * ta_info = Info();
      Info()->callback_id = JSWrap( std::function<void(std::string)>( [ta_info](std::string in_str){ta_info->DoCallback(in_str);} )  );
    }
    TextArea(const TextArea & in) : WidgetFacet(in) { ; }
    TextArea(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsTextAreaInfo()); }
    virtual ~TextArea() { ; }

    using INFO_TYPE = TextAreaInfo;

    bool GetDisabled() const { return Info()->disabled; }
    const std::string & GetText() const { return Info()->cur_text; }

    TextArea & SetAutofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }
    TextArea & SetCallback(const std::function<void(const std::string &)> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }
    TextArea & SetDisabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }
    TextArea & SetText(const std::string & in_text) { Info()->UpdateText(in_text); return *this; }
    
    bool HasAutofocus() const { return Info()->autofocus; }
    bool IsDisabled() const { return Info()->disabled; }
  };


}
}

#endif
