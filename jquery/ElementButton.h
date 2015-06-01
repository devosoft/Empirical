#ifndef EMP_JQ_ELEMENT_BUTTON_H
#define EMP_JQ_ELEMENT_BUTTON_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Managae a single button.
//

#include <functional>
#include <string>
#include <vector>

#include "emscripten.h"

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementButton : public Element {
  private:
    emp::JQ::Button base_info;

  public:
    ElementButton(const emp::JQ::Button & in_info, Element * in_parent)
      : Element(in_info.name, in_parent), base_info(in_info) { ; }
    ~ElementButton() { ; }

    // Do not allow Managers to be copied
    ElementButton(const ElementButton &) = delete;
    ElementButton & operator=(const ElementButton &) = delete;


    void UpdateNow() { ; }
    void PrintHTML(std::ostream & os) { ; }

  };

};
};

#endif
