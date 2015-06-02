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
      HTML.str("");
      HTML << "<img src=\"" << url << "\" alt=\"" << alt_text << "\"";
      if (width > 0 || height > 0) {
        HTML << " style=\"";
        if (width > 0) HTML << "width:" << width << "px;";
        if (height > 0) HTML << "height:" << height << "px;";
        HTML << "\"";
      }
      HTML << ">";
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
