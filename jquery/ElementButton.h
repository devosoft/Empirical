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
    void UpdateHTML() {
      HTML_string = "<button";
      if (title != "") {
        HTML_string += " title=\"";
        HTML_string += title;
        HTML_string += "\"";
      }
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
      HTML_string += " onclick=\"empCppCallback(";
      HTML_string += std::to_string(callback_id);
      HTML_string += ")\"";
        
      HTML_string += ">";
      HTML_string += label;
      HTML_string += "</button>";
    }

  public:
    ElementButton(const emp::JQ::Button & in_info, Element * in_parent)
      : Element(in_info.GetTempName(), in_parent), Button(in_info) { ; }
    ~ElementButton() { ; }

    // Do not allow Managers to be copied
    ElementButton(const ElementButton &) = delete;
    ElementButton & operator=(const ElementButton &) = delete;

  };

};
};

#endif
