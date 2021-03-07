#ifndef EMP_CODE_BLOCK_HPP
#define EMP_CODE_BLOCK_HPP

#include "../base/errors.hpp"
#include "../tools/string_utils.hpp"
#include "../web/Element.hpp"
#include "../web/Widget.hpp"

namespace emp {
namespace prefab {
  /**
   * Use CodeBlock class to easily add code to a web
   * application that is highlighted based on the language used.
   */
  class CodeBlock: public web::Element {
    private:
    // code element to hold user's code
      web::Element code{emp::to_string("code")};

    public:
      /**
       * List of all supported languages: https://highlightjs.org/static/demo/
       *
       * @note Due to JavaScript callback functions, can only set code for block
       * in constructor.
       *
       * @param code_block string of the code to be placed in code block
       * @param lang programming language to base syntax highlighting
       */
      CodeBlock(const std::string code_block, const std::string lang, const std::string & id="")
        : web::Element("pre", id) {
          this->SetAttr("class", lang);
          // trigger HighlightJS library to apply syntax highlighting
          this->RegisterUpdateJS([]() {
              emscripten_run_script("hljs.initHighlighting.called = false; hljs.initHighlighting();");
          });
          /*
           * make sure special characters (", ', &, <, >) appear as their symbol,
           * not rendered as html on web page
           */
          code << emp::to_web_safe_string(code_block);
          *this << code;
          }

      /*
       * TODO: << operator throws error
       * The method below throws an error when trying to stream
       * a code block into anything else (web element, html).
       *
       * template <typename T>
       * void operator<<(T invalid) {
       *   emp::LibraryError("Not allowed to add code to the code block after construction due to JavaScript callback order");
       * }
       */
  };
}
}

#endif
