#ifndef EMP_UI_TEXT_H
#define EMP_UI_TEXT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Text widget.
//


#include "Widget.h"

namespace emp {
namespace UI {

  class Text : public internal::Widget<Text> {
  public:
    Text(const std::string & in_name="") : Widget(in_name) { ; }
    ~Text() { ; }
  };

};
};

#endif
