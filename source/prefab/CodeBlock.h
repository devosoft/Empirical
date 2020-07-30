#ifndef EMP_CODE_BLOCK_H
#define EMP_CODE_BLOCK_H

#include "../web/Element.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"
#include "../base/errors.h"

namespace emp {
namespace prefab {
  /// Use CodeBlock class to easily add code to a web
  /// application that is highlighted based on the language used.
  class CodeBlock: public web::Element {
    private:
    // code element to hold user's code
      web::Element code{emp::to_string("code")};

    public:
      /// @param code_str string of the code to be placed in code block
      /// @param lang programming language to base syntax highlighting
      /// List of all supported languages: https://highlightjs.org/static/demo/
      /// Note: Due to JavaScript callback functions, can only set code for block
      /// in constructor.
      CodeBlock(const std::string code_str, const std::string lang, const std::string & in_name="")
        : web::Element("pre", in_name){
          this->SetAttr("class", lang);
          // trigger HighlightJS library to apply syntax highlighting
          this->DoUpdateJS([](){
              emscripten_run_script("hljs.initHighlighting.called = false; hljs.initHighlighting();");
          });
          // make sure special characters (", ', &, <, >) appear as their symbol,
          // not rendered as html on web page
          code << emp::to_web_safe_string(code_str);
          *this << code;
          }

      // TODO: << operator throw error
      // The method below throws an error when trying to stream
      // a code block into anything else (web element, html).
      // template <typename T>
      // void operator<<(T invalid){
      //     emp::LibraryError("Not allowed to add code to the code block after construction due to JavaScript callback order");
      // }
  };
}
}

#endif