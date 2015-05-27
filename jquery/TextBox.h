#ifndef EMP_JQ_TEXTBOX_H
#define EMP_JQ_TEXTBOX_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Handles a generic text box, with any number of paragraphs.
//

#include "Element.h"

namespace emp {
namespace JQ {

  class TextBox : public Element {
  protected:
    std::string text;
  public:
    TextBox(const std::string & in_name, const std::string & in_text="")
      : Element(in_name), text(in_text) { ; }
    ~TextBox() { ; }

    const std::string & GetText() const { return text; }
    
    void SetText(const std::string & in_text) { text = in_text; up_to_date = false; }
    void Append(const std::string & in_text) { text += in_text; up_to_date = false; }
  };

};
};

#endif
