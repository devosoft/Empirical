#ifndef EMP_UI_ELEMENT_WRAPPER_H
#define EMP_UI_ELEMENT_WRAPPER_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Wrapper to build an specialized HTML element.
//

#include "Element.h"

namespace emp {
namespace UI {

  template <typename BASE_TYPE>
  class ElementWrapper : public Element, public BASE_TYPE {
  private:
    void UpdateHTML() {
      HTML.str("");                               // Clear the current stream.
      BASE_TYPE::WriteHTML(HTML);
    }

    void UpdateCSS() {
      BASE_TYPE::TriggerCSS();
    }

  public:
    ElementWrapper(const BASE_TYPE & in_info, Element * in_parent)
      : Element(in_info.GetDivID(), in_parent), BASE_TYPE(in_info) { ; }

    BASE_TYPE & DivID(const std::string & in_name) { 
      emp_assert(false && "Cannot change div ID after div is created.");
      return *this;
    }
  };

};
};

#endif

