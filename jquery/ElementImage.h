#ifndef EMP_JQ_ELEMENT_IMAGE_H
#define EMP_JQ_ELEMENT_IMAGE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a single image.
//

#include <string>

#include "emscripten.h"

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementImage : public Element, public emp::JQ::Image {
  private:
    void UpdateHTML() {
      HTML_string = "<img src='";
      HTML_string += url;
      HTML_string += "' alt='";
      HTML_string += alt_text;
      HTML_string += "'";
      if (width > 0 || height > 0) {
        HTML_string += " style='";
        if (width > 0) {
          HTML_string += "width:";
          HTML_string += std::to_string(width);
          HTML_string += "px;";
        }
        if (height > 0) {
          HTML_string += "height:";
          HTML_string += std::to_string(height);
          HTML_string += "px;";
        }
        HTML_string += "'";
      }
      HTML_string += ">";
    }

  public:
    ElementImage(const emp::JQ::Image & in_info, Element * in_parent)
      : Element(in_info.GetTempName(), in_parent), emp::JQ::Image(in_info) { ; }
    ~ElementImage() { ; }

    // Do not allow Managers to be copied
    ElementImage(const ElementImage &) = delete;
    ElementImage & operator=(const ElementImage &) = delete;

  };

};
};

#endif
