#ifndef EMP_JQ_ELEMENT_TEXT_H
#define EMP_JQ_ELEMENT_TEXT_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Manage a section of the current web page
//

#include <string>
#include <sstream>

#include "emscripten.h"

#include "Element.h"

namespace emp {
namespace JQ {

  class ElementText : public Element {
  private:
    std::stringstream text;
  public:
    ElementText(const std::string & name, Element * in_parent) : Element(name, in_parent) { ; }
    ~ElementText() { ; }

    // Do not allow Managers to be copied
    ElementText(const ElementText &) = delete;
    ElementText & operator=(const ElementText &) = delete;

    void ClearText() { text.str(std::string()); }
    void AppendText(const std::string & in_text) { text << in_text; }

    template <typename IN_TYPE>
    void Append(const IN_TYPE & in_var) { text << in_var; }

    virtual void PrintHTML(std::ostream & os) {
      os << text.str() << std::endl;
    }
  };

};
};

#endif
