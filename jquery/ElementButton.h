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
      HTML.str("");                               // Clear the current stream.
      HTML << "<button";                          // Start the button tag.
      if (title != "") {                          // If the button has a title, add it!
        HTML << " title=\"" << title << "\"";
      }
      if (width > 0 || height > 0) {              // If the button has a style, add it!
        HTML << " style=\"";
        if (width > 0) HTML << "width:" << width << "px;";
        if (height > 0) HTML << "height:" << height << "px;";
        HTML << "\"";
      }

      // Indicate action on click.
      HTML << " onclick=\"empCppCallback(" << std::to_string(callback_id) << ")\"";

      HTML << ">" << label << "</button>";       // Close and label the button.
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
