#ifndef EMP_UI_TEXT_H
#define EMP_UI_TEXT_H


//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Text widget.
//


#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Text_detail class
  namespace internal { class Text_detail; };

  // Specify the Text class for use in return values in the Text_detail definitions below.
  using Text = internal::Widget_wrap<internal::Text_detail>;
  
  // Define Text_detail in internal namespace (i.e., details should not be accessed directly.)
  namespace internal {
    class Text_detail : public Widget_base {
    public:
      Text_detail() { ; }
      ~Text_detail() { ; }
    };
  };

};
};

#endif
