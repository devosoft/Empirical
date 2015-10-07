#ifndef EMP_WEB_FILE_INPUT_H
#define EMP_WEB_FILE_INPUT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the FileInput widget (click on to upload a file)
//
//  FileInput will convert the file to a std::string and pass the result to a designated
//  function.
//

#include "Widget.h"

namespace emp {
namespace web {

  class FileInput : public internal::WidgetFacet<FileInput> {
    friend class FileInputInfo;
  protected:

    // FileInputs associated with the same DOM element share a single FileInputInfo object.
    class FileInputInfo : public internal::WidgetInfo {
      friend FileInput;
    protected:
      std::string title;
      
      bool autofocus;
      bool disabled;
      
      std::function<void(const std::string &)> callback;
      uint32_t callback_id;
      
      FileInputInfo(const std::string & in_id="") : internal::WidgetInfo(in_id) { ; }
      FileInputInfo(const FileInputInfo &) = delete;               // No copies of INFO allowed
      FileInputInfo & operator=(const FileInputInfo &) = delete;   // No copies of INFO allowed
      virtual ~FileInputInfo() {
        if (callback_id) emp::JSDelete(callback_id);               // Delete callback wrapper.
      }
      
      void DoCallback() {
        function handleFileSelect(evt) {
          var infile = evt.target.files; // FileList object (List of File objects)

          var reader = new FileReader();
          reader.onload = function(e) {
            emkLoadString(e.target.result + '\n');
          };
          reader.readAsText(evt.target.files.item(0));
        }

        
        callback();
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                           // Clear the current text.
        HTML <<"<input type=\"file\"";
        if (title != "") HTML << " title=\"" << title << "\"";  // Add a title if there is one.
        if (disabled) { HTML << " disabled=true"; }             // Check if should be disabled
        HTML << " id=\"" << id << "\"";                         // Indicate ID.
        HTML << " name=\"" << id << "\"";                       // Use same name as ID.
        HTML << " onchange=\"" << emp::to_string("emp.Callback(", callback_id, ")") << "\"";

        HTML << ">";
      }
      
      void UpdateCallback(const std::function<void(const std::string &)> & in_cb) {
        callback = in_cb;
      }

      void UpdateTitle(const std::string & in_title) {
        title = in_title;
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
      virtual std::string GetType() override { return "web::FileInputInfo"; }
    }; // End of FileInputInfo definition
    
    
    // Get a properly cast version of indo.  
    FileInputInfo * Info() { return (FileInputInfo *) info; }
    const FileInputInfo * Info() const { return (FileInputInfo *) info; }
 
    FileInput(FileInputInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    FileInput(const std::function<void(const std::string &)> & in_cb,
              const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new FileInputInfo(in_id);
    
      Info()->title = "";
      Info()->autofocus = false;
      Info()->disabled = false;
      
      Info()->callback = in_cb;
      FileInputInfo * b_info = Info();
      Info()->callback_id = JSWrap( std::function<void()>( [b_info](){b_info->DoCallback();} )  );
    }
    FileInput(const FileInput & in) : WidgetFacet(in) { ; }
    FileInput(const Widget & in) : WidgetFacet(in) { emp_assert(info->IsFileInputInfo()); }
    virtual ~FileInput() { ; }

    using INFO_TYPE = FileInputInfo;

    FileInput & Callback(const std::function<void(const std::string &)> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }
    FileInput & Title(const std::string & in_t) { Info()->UpdateTitle(in_t); return *this; }
    FileInput & Autofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }
    FileInput & Disabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }
    
    const std::string & GetTitle() const { return Info()->title; }
    bool HasAutofocus() const { return Info()->autofocus; }
    bool IsDisabled() const { return Info()->disabled; }
  };


}
}

#endif
