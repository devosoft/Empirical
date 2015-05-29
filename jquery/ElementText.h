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

    virtual bool IsText() const { return true; }

    void ClearText() { text.str(std::string()); }
    void AppendText(const std::string & in_text) { text << in_text; }

    Element & Append(const std::string & in_text) {
      text << in_text;
      SetModified();
      return *this;
    }

    void UpdateNow() {
      EM_ASM_ARGS({
          var elem_name = Pointer_stringify($0);
          var text = Pointer_stringify($1);
          $( '#' + elem_name ).html(text);
        }, GetName().c_str(), text.str().c_str() );
    }

    virtual void PrintHTML(std::ostream & os) {
      os << text.str() << std::endl;
    }
  };

};
};

#endif
