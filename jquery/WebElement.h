#ifndef EMP_WEB_ELEMENT_H
#define EMP_WEB_ELEMENT_H

////////////////////////////////////////////////////////////////////////////////////
//
//  A single element on a web page (a paragraph, a table, etc.)
//

#include <emscripten>
#include <string>

namespace emp {

  class WebElement {
  private:
    std::string name;

  public:
    WebElement(const std::string & in_name) : name(in_name) { ; }
    WebElement(const WebElement &) = delete;
    WebElement & operator=(const WebElement &) = delete;

    void SetText(const std::string & text) {
      EM_ASM_ARGS({
          var element_name = Pointer_stringify($0);
          var new_text = Pointer_stringify($1);
          $( element_name ).html( new_text );
      }, name.c_str(), text.c_str());
    }
  };

};

#endif
