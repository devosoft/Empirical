#ifndef EMP_JQ_ELEMENT_WRAPPER_H
#define EMP_JQ_ELEMENT_WRAPPER_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper to build an specialized HTML element.
//

#include "Element.h"

namespace emp {
namespace JQ {

  template <typename BASE_TYPE>
  class ElementWrapper : public Element, public BASE_TYPE {
  private:
    void UpdateHTML() {
      HTML.str("");                               // Clear the current stream.
      BASE_TYPE::WriteHTML(HTML);
    }

  public:
    ElementWrapper(const BASE_TYPE & in_info, Element * in_parent)
      : Element(in_info.GetTempName(), in_parent), BASE_TYPE(in_info) { ; }
  };

};
};

#endif

