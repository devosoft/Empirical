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

  public:
    ElementImage(const emp::JQ::Image & in_info, Element * in_parent)
      : Element(in_info.name, in_parent), base_info(in_info) { ; }
    ~ElementImage() { ; }

    // Do not allow Managers to be copied
    ElementImage(const ElementImage &) = delete;
    ElementImage & operator=(const ElementImage &) = delete;

    void UpdateNow() { ; }
    void PrintHTML(std::ostream & os) { ; }

  };

};
};

#endif
