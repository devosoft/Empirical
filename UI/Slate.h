#ifndef EMP_UI_SLATE_H
#define EMP_UI_SLATE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Slate widget.
//


#include "Widget.h"

namespace emp {
namespace UI {

  class Slate : public internal::Widget<Slate> {
  public:
    Slate(const std::string & in_name) : Widget(in_name) { ; }
    ~Slate() { ; }
  };

};
};

#endif
