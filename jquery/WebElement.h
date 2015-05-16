#ifndef EMP_WEB_ELEMENT_H
#define EMP_WEB_ELEMENT_H

////////////////////////////////////////////////////////////////////////////////////
//
//  A single element on a web page (a paragraph, a table, etc.)
//

#include <emscripten.h>
#include <string>

#include "../tools/assert.h"

namespace emp {

  class WebElement {
  private:
    std::string name;

  public:
    WebElement(const std::string & in_name) : name(in_name) {
      emp_assert(name.size() > 0);  // Make sure the name exists!
      // @CAO ensure the name consists of just alphanumeric chars (plus '_' & '-'?)
    }
    WebElement(const WebElement &) = delete;
    virtual ~WebElement() { ; }
    WebElement & operator=(const WebElement &) = delete;

    const std::string GetName() { return name; }

    virtual void SetText(const std::string & text) {
      EM_ASM_ARGS({
          var element_name = Pointer_stringify($0);
          var new_text = Pointer_stringify($1);
          $( '#' + element_name ).html( new_text );
      }, name.c_str(), text.c_str());
    }
  };

};

#endif
