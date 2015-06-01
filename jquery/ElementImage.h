#ifndef EMP_JQ_ELEMENT_IMAGE_H
#define EMP_JQ_ELEMENT_IMAGE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a single image.
//

#include <functional>
#include <string>
#include <vector>

#include "emscripten.h"

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementImage : public Element {
  private:
    emp::JQ::Image base_info;
    std::string HTML_string;

    void UpdateHTML() {
      HTML_string = "<img src='";
      HTML_string += base_info.url;
      HTML_string += "' alt='";
      HTML_string += base_info.alt_text;
      HTML_string += "'";
      if (base_info.width > 0 || base_info.height > 0) {
        HTML_string += " style='";
        if (base_info.width > 0) {
          HTML_string += "width:";
          HTML_string += std::to_string(base_info.width);
          HTML_string += "px;";
        }
        if (base_info.height > 0) {
          HTML_string += "height:";
          HTML_string += std::to_string(base_info.height);
          HTML_string += "px;";
        }
        HTML_string += "'";
      }
      HTML_string += ">";
    }

  public:
    ElementImage(const emp::JQ::Image & in_info, Element * in_parent)
      : Element(in_info.name, in_parent), base_info(in_info) { ; }
    ~ElementImage() { ; }

    // Do not allow Managers to be copied
    ElementImage(const ElementImage &) = delete;
    ElementImage & operator=(const ElementImage &) = delete;

    void UpdateNow() {
      UpdateHTML();
      EM_ASM_ARGS({
          var elem_name = Pointer_stringify($0);
          var html_str = Pointer_stringify($1);
          $( '#' + elem_name ).html(html_str);
        }, GetName().c_str(), HTML_string.c_str() );
    }

    void PrintHTML(std::ostream & os) {
      UpdateHTML();
      os << HTML_string;
    }

  };

};
};

#endif
