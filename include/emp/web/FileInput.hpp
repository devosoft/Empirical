/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2015-2022 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/web/FileInput.hpp
 * @brief Read a browser-selected file through a callback.
 *
 * Use example:
 *
 *   emp::web::FileInput upload([](const emp::web::FileData & file) {
 *     ProcessFile(file.contents);
 *   }, "upload");
 *   upload.Accept(".txt,.csv").OnError(ReportError);
 *   doc << upload;
 *
 * Here doc is a web::Document or another container. FileData supplies the file's
 * name, MIME type, and exact bytes, including embedded NULs. OnError() receives
 * a diagnostic string if reading fails. Legacy callbacks taking const std::string &
 * receive decoded text with a trailing newline; callbacks taking emp::File
 * receive parsed lines. Optional trailing classes style the input. Selecting the
 * same file again triggers another read.
 *
 * @todo Setup FileInput to work outside of web mode as well.
 */

#pragma once

#ifndef INCLUDE_EMP_WEB_FILE_INPUT_HPP_GUARD
#define INCLUDE_EMP_WEB_FILE_INPUT_HPP_GUARD

#include <cstdint>
#include <expected>
#include <functional>
#include <string>

#include "../io/File.hpp"

#include "Widget.hpp"

namespace emp::web {

  ///  FileInput will convert the file to a std::string and pass the result to a
  ///  designated function.
  ///
  ///  To create a new file input, you must pass it a void function that takes a
  ///  const std::string & as its only argument.  When a file is loaded, the
  ///  specified function is called and the body of the file is passed in as the
  ///  string.

  struct FileData {
    std::string name;
    std::string mime_type;
    std::string contents; ///< Exact bytes, including embedded NULs; UTF-8 text is unchanged.
  };

  class FileInput : public internal::WidgetFacet<FileInput> {
    friend class FileInputInfo;
  protected:
// FileInputs associated with the same DOM element share a single FileInputInfo object.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    class FileInputInfo : public internal::WidgetInfo {
      friend FileInput;
    protected:
      bool autofocus;
      bool disabled;

      std::function<void(const std::string &)> callback;
      std::function<void(const FileData &)> file_callback;
      std::function<void(const std::string &)> error_callback;
      JSCallback data_callback;
      bool exact_bytes = false;

      FileInputInfo(const std::string & in_id = "") : internal::WidgetInfo(in_id) { ; }

      FileInputInfo(const FileInputInfo &)             = delete;  // No copies of INFO allowed
      FileInputInfo & operator=(const FileInputInfo &) = delete;  // No copies of INFO allowed

      virtual ~FileInputInfo() {
      }

      std::string GetTypeName() const override { return "FileInputInfo"; }

      void DoCallback(const std::string & file_body) {
        Widget keep_alive(this); // User callbacks may remove the final external widget handle.
        callback(file_body);
        UpdateDependants();
      }

      virtual void GetHTML(std::stringstream & HTML) override {
        HTML.str("");  // Clear the current text.
        HTML << "<input type=\"file\"";
        if (disabled) { HTML << " disabled=true"; }  // Check if should be disabled
        HTML << " id=\"" << id << "\"";              // Indicate ID.
        HTML << " name=\"" << id << "\"";            // Use same name as ID
        HTML << " onchange=\"emp.ReadFileInput(this, " << data_callback.GetID()
             << ", " << (exact_bytes ? "true" : "false") << ")\"";
        HTML << ">";
      }

      void UpdateCallback(const std::function<void(const std::string &)> & in_cb) {
        callback = in_cb;
      }

      void UpdateAutofocus(bool in_af) {
        autofocus = in_af;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

      void UpdateDisabled(bool in_dis) {
        disabled = in_dis;
        if (state == Widget::ACTIVE) {
          ReplaceHTML();  // If node is active, immediately redraw!
        }
      }

    public:
      virtual std::string GetType() override { return "web::FileInputInfo"; }
    };  // End of FileInputInfo definition
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS

    // Get a properly cast version of info.
    FileInputInfo * Info() { return (FileInputInfo *) info; }

    const FileInputInfo * Info() const { return (FileInputInfo *) info; }

    FileInput(FileInputInfo * in_info) : WidgetFacet(in_info) { ; }

  public:
    /// Create a new FileInput; supply the function to call with the file contents as a string
    /// (and optionally the HTML identifier to be used).
    FileInput(const std::function<void(const std::string &)> & in_cb,
              const std::string & in_id = "", const std::string & classes = "")
      : WidgetFacet(in_id) {
      info = new FileInputInfo(in_id);
      AddClass(classes);

      Info()->autofocus = false;
      Info()->disabled  = false;

      Info()->callback       = in_cb;
      FileInputInfo * w_info = Info();
      Info()->data_callback = JSCallback([w_info](size_t address, size_t size, std::string name,
                                                  std::string mime, std::string error) {
        Widget keep_alive(w_info);
        if (!error.empty()) {
          if (w_info->error_callback) { w_info->error_callback(error); }
          return;
        }
        const auto * data = reinterpret_cast<const char *>(address);
        FileData file{std::move(name), std::move(mime), std::string(data, size)};
        if (w_info->exact_bytes) {
          auto fn = w_info->file_callback;
          if (fn) { fn(file); }
          w_info->UpdateDependants();
        } else {
          // Preserve the historical text callback's trailing newline.
          w_info->DoCallback(file.contents + "\n");
        }
      });
    }

    /// Create a new FileInput; supply the function to call with the file contents as a File object
    /// (and optionally the HTML identifier to be used).
    FileInput(const std::function<void(emp::File)> & cb, const std::string & in_id = "")
      : FileInput(
          [cb](const std::string & in) {
            std::stringstream ss(in);
            File file(ss);
            cb(file);
          },
          in_id) {
      ;
    }

    /// Exact binary/text reads with filename and MIME type; errors go to OnError().
    FileInput(const std::function<void(const FileData &)> & callback,
              const std::string & id = "", const std::string & classes = "")
      : FileInput(std::function<void(const std::string &)>{[](const std::string &) {}}, id, classes) {
      Info()->exact_bytes = true;
      Info()->file_callback = callback;
    }

    FileInput & OnError(const std::function<void(const std::string &)> & callback) {
      Info()->error_callback = callback;
      return *this;
    }

    FileInput & Accept(const std::string & types) { return SetAttr("accept", types); }

    /// Load a pre-existing FileInput object.
    FileInput(const FileInput & in) = default;

    FileInput(const Widget & in) : WidgetFacet(in) { ; }

    virtual ~FileInput() = default;

    using INFO_TYPE = FileInputInfo;

    /// Change the callback function to use when a new file is loaded.
    FileInput & Callback(const std::function<void(const std::string &)> & in_cb) {
      Info()->UpdateCallback(in_cb);
      Info()->exact_bytes = false;
      if (IsActive()) { Info()->ReplaceHTML(); }
      return *this;
    }

    /// Set this FileInput object to have autofocus (or not)
    FileInput & Autofocus(bool in_af) {
      Info()->UpdateAutofocus(in_af);
      return *this;
    }

    /// Set this FileInput object to be disabled (or reenable it.)
    FileInput & Disabled(bool in_dis) {
      Info()->UpdateDisabled(in_dis);
      return *this;
    }

    /// Determine if this object currently has autofocus.
    bool HasAutofocus() const { return Info()->autofocus; }

    /// Determine if this object is currently disabled.
    bool IsDisabled() const { return Info()->disabled; }
  };


}  // namespace emp::web

#endif  // #ifndef INCLUDE_EMP_WEB_FILE_INPUT_HPP_GUARD
