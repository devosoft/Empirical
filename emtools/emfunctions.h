#ifndef EMP_EM_FUNCTIONS_H
#define EMP_EM_FUNCTIONS_H

#include <functional>

#include "../tools/alert.h"
#include "../tools/string_utils.h"
#include "JSWrap.h"

namespace emp {

#define AlertVar(VAR) emp::Alert(std::string(#VAR) + std::string("=") + std::to_string(VAR))

  template<class T> void DelayCall(const std::function<void()> & in_fun, int delay) {
    uint32_t callback_id = JSWrapOnce(in_fun); // Wrap and dispose when called.
    EM_ASM_ARGS({ window.setTimeout(function() { emp.Callback($0); }, $1); }, callback_id, delay);
  }

  int GetWindowInnerWidth() { return EM_ASM_INT_V({ return window.innerWidth; }); }
  int GetWindowInnerHeight() { return EM_ASM_INT_V({ return window.innerHeight; }); }

  void SetBackgroundColor(const std::string color) {
    EM_ASM_ARGS({
        var color = Pointer_stringify($0);
        $("body").first().css("background-color", color);
      }, color.c_str());
  }
  
  void SetColor(const std::string color) {
    EM_ASM_ARGS({
        var color = Pointer_stringify($0);
        $("body").first().css("color", color);
      }, color.c_str());
  }
  
  // These may already be in HTML5 for Emscripten
  void SetCursor(const char * type) {
    EM_ASM_ARGS({
        var type = Pointer_stringify($0);
        document.body.style.cursor = type;
    }, type);
  }

  void OpenWindow(const std::string & url) {
    EM_ASM_ARGS({
        var url = Pointer_stringify($0);
        window.open = url;
    }, url.c_str());
  }

  // Convert a sequence with possible html codes to appear identically in html.
  std::string text2html(const std::string & text) {
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
};

#endif
