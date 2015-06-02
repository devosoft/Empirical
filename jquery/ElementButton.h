#ifndef EMP_JQ_ELEMENT_BUTTON_H
#define EMP_JQ_ELEMENT_BUTTON_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Managae a single button.
//

#include <string>

#include "emscripten.h"

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementButton : public Element, public Button {
  private:
    std::string HTML_string;

    void UpdateHTML() {
      HTML_string = "<img src='";
    }

  public:
    ElementButton(const emp::JQ::Button & in_info, Element * in_parent)
      : Element(in_info.GetTempName(), in_parent), Button(in_info) { ; }
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
