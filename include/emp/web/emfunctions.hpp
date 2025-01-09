/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2017
*/
/**
 *  @file
 *  @brief Specialized, useful function for Empirical.
 */

#ifndef EMP_WEB_EMFUNCTIONS_HPP_INCLUDE
#define EMP_WEB_EMFUNCTIONS_HPP_INCLUDE

#include <cstdint>
#include <functional>

#include "../debug/alert.hpp"
#include "../tools/string_utils.hpp"

#include "JSWrap.hpp"

namespace emp {

#define AlertVar(VAR) emp::Alert(std::string(#VAR) + std::string("=") + std::to_string(VAR))

  /// Call a function after a specified amount of time (in milliseconds)
  static void DelayCall(const std::function<void()> & in_fun, int delay) {
    uint32_t callback_id = JSWrapOnce(in_fun); // Wrap and dispose when called.
    (void)callback_id;
    MAIN_THREAD_ASYNC_EM_ASM({
        window.setTimeout(function() { emp.Callback($0); }, $1);
      }, callback_id, delay);
  }

  /// Provide a function to call whenever a window's size changes (no arguments).
  static void OnResize(const std::function<void()> & in_fun) {
    uint32_t callback_id = JSWrap(in_fun);
    MAIN_THREAD_ASYNC_EM_ASM({
        window.addEventListener("resize", function() { emp.Callback($0); });
      }, callback_id);
  }

  /// Provide a function to call whenever a window's size changes (new size as arguments)
  static void OnResize(const std::function<void(int,int)> & in_fun) {
    uint32_t callback_id = JSWrap(in_fun);
    (void)callback_id;
    MAIN_THREAD_ASYNC_EM_ASM({
        window.addEventListener("resize", function() {
            emp.Callback($0, window.innerWidth, window.innerHeight);
          });
      }, callback_id);
  }

  /// Get the current time, as provided by the web browser.
  inline double GetTime() { return MAIN_THREAD_EM_ASM_DOUBLE({ return (new Date()).getTime(); }); }

  /// Determine with width of the current window.
  inline int GetWindowInnerWidth() { return MAIN_THREAD_EM_ASM_INT({ return window.innerWidth; }); }

  /// Determine with height of the current window.
  inline int GetWindowInnerHeight() { return MAIN_THREAD_EM_ASM_INT({ return window.innerHeight; }); }

  /// Set the background color of this web page.
  static void SetBackgroundColor(const std::string color) {
    MAIN_THREAD_ASYNC_EM_ASM({
        var color = UTF8ToString($0);
        document.body.style.backgroundColor = color;
      }, color.c_str());
  }

  static void SetColor(const std::string color) {
    MAIN_THREAD_ASYNC_EM_ASM({
        var color = UTF8ToString($0);
        document.body.style.color = color;
      }, color.c_str());
  }

  // These may already be in HTML5 for Emscripten
  static void SetCursor(const char * type) {
    MAIN_THREAD_ASYNC_EM_ASM({
        var type = UTF8ToString($0);
        document.body.style.cursor = type;
    }, type);
  }

  static void OpenWindow(const std::string & url) {
    MAIN_THREAD_ASYNC_EM_ASM({
        var url = UTF8ToString($0);
        window.open = url;
    }, url.c_str());
  }

  // Convert a sequence with possible html codes to appear identically in html.
  static std::string text2html(const std::string & text) {
    std::stringstream html;
    for (char x : text) {
      switch (x) {
      case '<': html << "&lt;"; break;
      case '>': html << "&gt;"; break;
      case '&': html << "&amp;"; break;
      case ' ': html << "&nbsp;"; break;
      case '\n': html << "<br>"; break;
      default: html << x;
      };
    }
    return html.str();
  }

  emp::String MakeHTMLLink(emp::String text, emp::String link, emp::String color="") {
    emp::String style;
    if (color.size()) style += emp::MakeString("color: ", color, "; ");
    if (style.size()) style = emp::MakeString(" style=\"", style, "\"");
    return emp::MakeString("<a href=\"", link, "\"", style, ">", text, "</a>");
  }

  /// @brief Generate a string that will associate text with a clickable link to call a function.
  /// @param text The text you want in the link.
  /// @param in_fun The function to call when the text is clicked on.
  /// @return The generated string
  /// NOTE: This will wrap a new function each time you call, so minimize calls!
  emp::String MakeHTMLTrigger(emp::String text, std::function<void()> in_fun, emp::String color="") {
    emp::String style;
    if (color.size()) style += emp::MakeString("color: ", color, "; ");
    if (style.size()) style = emp::MakeString(" style=\"", style, "\"");
    uint32_t callback_id = JSWrap(in_fun);
    return MakeString("<a href=\"#\" onclick=\"emp.Callback(", callback_id, "); return false;\"",
                      style, ">", text, "</a>");
  }

  /// Get the value of @param attribute in the element with @param id as its id.
  inline std::string GetElementAttribute(const std::string & id, const std::string & attribute) {
    char * buffer = (char * )MAIN_THREAD_EM_ASM_INT({
      var text = document.getElementById(UTF8ToString($0))[UTF8ToString($1)];
      var buffer = Module._malloc(text.length+1);
      Module.stringToUTF8(text, buffer, text.length*4+1);
      return buffer;
    }, id.c_str(), attribute.c_str());

    std::string result = std::string(buffer);
    free(buffer);
    return result;
  }

  // Place text into the user's clipboard.  Return success.
  inline void CopyText(const std::string & text) {
    MAIN_THREAD_ASYNC_EM_ASM({
      const copy_text = UTF8ToString($0);
      navigator.clipboard.writeText(copy_text)
        .catch(err => { alert("Unable to copy text to clipboard. Is your connection secure?"); });
    }, text.c_str());
  }

  // Generate a file that the user can then download.
  inline void DownloadFile(std::string filename, const std::string & content) {    
    MAIN_THREAD_ASYNC_EM_ASM({
      var filename = UTF8ToString($0);
      var content = UTF8ToString($1);
      const blob = new Blob([content], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = filename;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
    }, filename.c_str(), content.c_str());
  }

}

#endif // #ifndef EMP_WEB_EMFUNCTIONS_HPP_INCLUDE
