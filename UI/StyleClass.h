#ifndef EMP_UI_STYLE_CLASS_H
#define EMP_UI_STYLE_CLASS_H

///////////////////////////////////////////////////////////////////////////////////////////
//
//  A traditional CSS class that can modify groups of elements
//

#include "Style.h"

namespace emp {
namespace UI {

  class StyleClass : public Style {
  private:
    std::string name;  // What is the name of this class?  (.classname)

  public:
  };

};
};

/*
document.styleSheets[0].cssRules[0].cssText = "\
     #myID {
         myRule: myValue;
         myOtherRule: myOtherValue;
     }";
*/

#endif
