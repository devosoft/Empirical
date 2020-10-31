/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2018
 *
 *  @file  FileInput.hpp
 *  @brief Specs for the FileInput widget (click on to upload a file)
 *
 *  @todo Setup FileInput to work outside of web mode as well.
 */

#ifndef EMP_WEB_FILE_INPUT_H
#define EMP_WEB_FILE_INPUT_H

#include <functional>
#include <string>

#include "../io/File.hpp"
#include "Widget.hpp"

namespace emp {
namespace web {

  ///  FileInput will convert the file to a std::string and pass the result to a
  ///  designated function.
  ///
  ///  To create a new file input, you must pass it a void function that takes a
  ///  const std::string & as its only argument.  When a file is loaded, the
  ///  specified function is called and the body of the file is passed in as the
  ///  string.

  class FileInput : public internal::WidgetFacet<FileInput> {
    friend class FileInputInfo;
  protected:

    // FileInputs associated with the same DOM element share a single FileInputInfo object.
    class FileInputInfo : public internal::WidgetInfo {
      friend FileInput;
    protected:
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

      std::string GetTypeName() const override { return "FileInputInfo"; }

      void DoCallback(const std::string & file_body) {
        callback(file_body);
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");                                             // Clear the current text.
        HTML <<"<input type=\"file\"";
        if (disabled) { HTML << " disabled=true"; }               // Check if should be disabled
        HTML << " id=\"" << id << "\"";                           // Indicate ID.
        HTML << " name=\"" << id << "\"";                         // Use same name as ID
        HTML << " onchange=\"emp.LoadFileEvent(this.files, " << callback_id <<  ")\"";
        HTML << ">";
      }

      void UpdateCallback(const std::function<void(const std::string &)> & in_cb) {
        callback = in_cb;
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
    /// Create a new Fileinput; supply the function to call with the file contents as a string
    /// (and optionally the HTML identifier to be used).
    FileInput(const std::function<void(const std::string &)> & in_cb, const std::string & in_id="")
      : WidgetFacet(in_id)
    {
      info = new FileInputInfo(in_id);

      Info()->autofocus = false;
      Info()->disabled = false;

      Info()->callback = in_cb;
      FileInputInfo * w_info = Info();
      using callback_t = std::function<void(const std::string & file_body)>;
      Info()->callback_id = JSWrap( callback_t( [w_info](const std::string & file_body){w_info->DoCallback(file_body);} )  );
    }

    /// Create a new FileInput; supply the function to call with the file contents as a File object
    /// (and optionally the HTML identifier to be used).
    FileInput(const std::function<void(const emp::File &)> & cb, const std::string & in_id="")
      : FileInput( [cb](const std::string & in){ std::stringstream ss(in); File file(ss); cb(file); } ) { ; }

    /// Load a pre-existing FileInput object.
    FileInput(const FileInput & in) : WidgetFacet(in) { ; }
    FileInput(const Widget & in) : WidgetFacet(in) { ; }
    virtual ~FileInput() { ; }

    using INFO_TYPE = FileInputInfo;

    /// Change the callback function to use when a new file is loaded.
    FileInput & Callback(const std::function<void(const std::string &)> & in_cb) {
      Info()->UpdateCallback(in_cb);
      return *this;
    }

    /// Set this FileInput object to have autofocus (or not)
    FileInput & Autofocus(bool in_af) { Info()->UpdateAutofocus(in_af); return *this; }

    /// Set this FileInput object to be disabled (or renable it.)
    FileInput & Disabled(bool in_dis) { Info()->UpdateDisabled(in_dis); return *this; }

    /// Determine if this object currently has autofocus.
    bool HasAutofocus() const { return Info()->autofocus; }

    /// Determine if this object is currently disabled.
    bool IsDisabled() const { return Info()->disabled; }
  };


}
}

#endif
