#ifndef EMP_UI_ELEMENT_TEXT_H
#define EMP_UI_ELEMENT_TEXT_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <functional>
#include <string>
#include <vector>

#include <emscripten.h>

#include "../tools/DynamicStringSet.h"

#include "Element.h"

namespace emp {
namespace UI {

  class ElementText : public Element {
  private:
    DynamicStringSet strings;

    void UpdateHTML() {
      HTML.str("");        // Clear the current text.
      HTML << strings;  // Save the current value of the strings.
    }

  public:
    ElementText(const std::string & in_name, Element * in_parent)
      : Element(in_name, in_parent) { ; }
    ~ElementText() { ; }

    // Do not allow Managers to be copied
    ElementText(const ElementText &) = delete;
    ElementText & operator=(const ElementText &) = delete;

    virtual bool IsText() const { return true; }

    void ClearText() { strings.Clear(); }

    Element & Append(const std::string & in_text) {
      strings.Append(in_text);
      return *this;
    }

    Element & Append(const std::function<std::string()> & in_fun) {
      strings.Append(in_fun);
      return *this;
    }

  };

};
};

#endif
