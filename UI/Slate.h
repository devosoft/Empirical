#ifndef EMP_UI_SLATE_H
#define EMP_UI_SLATE_H


//////////////////////////////////////////////////////////////////////////////////////////
//
//  Specs for the Slate widget.
//


#include "UI_base.h"

namespace emp {
namespace UI {

  // Forward-declare the internal Slate_detail class
  namespace internal { class Slate_detail; };

  // Specify the Slate class for use in return values in the Slate_detail definitions below.
  using Slate = internal::Widget_wrap<internal::Slate_detail>;
  
  // Define Slate_detail in internal namespace (i.e., details should not be accessed directly.)
  namespace internal {
    class Slate_detail : public Widget_base {
    public:
      Slate_detail() { ; }
      ~Slate_detail() { ; }
    };
  };

};
};

#endif
